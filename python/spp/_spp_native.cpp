#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "BLEU.h"
#include "spp.h"

#include <exception>
#include <sstream>
#include <memory>
#include <utility>
#include <vector>

namespace {

struct OwnedItems {
	std::vector<StripPacking::item*> raw;

	~OwnedItems() {
		for (auto* it : raw) {
			delete it;
		}
	}
};

PyObject* pack_native(PyObject* /*self*/, PyObject* args, PyObject* kwargs) {
	static const char* kwlist[] = {"items", "bin_width", "branch_and_bound", "benders", "timeout", nullptr};
	PyObject* items_obj = nullptr;
	int bin_width = 0;
	int branch_and_bound = 1;
	int benders = 1;
	int timeout = 60;
	if (!PyArg_ParseTupleAndKeywords(
			args,
			kwargs,
			"Oippi:pack_native",
			const_cast<char**>(kwlist),
			&items_obj,
			&bin_width,
			&branch_and_bound,
			&benders,
			&timeout)) {
		return nullptr;
	}

	if (bin_width <= 0) {
		PyErr_SetString(PyExc_ValueError, "bin_width must be a positive integer");
		return nullptr;
	}
	if (timeout < 0) {
		PyErr_SetString(PyExc_ValueError, "timeout must be a non-negative integer");
		return nullptr;
	}
	if (!branch_and_bound && !benders) {
		PyErr_SetString(PyExc_ValueError, "At least one of branch_and_bound and benders must be True");
		return nullptr;
	}

	PyObject* seq = PySequence_Fast(items_obj, "items must be a sequence of (width, height) tuples");
	if (seq == nullptr) {
		return nullptr;
	}
	const Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
	PyObject** seq_items = PySequence_Fast_ITEMS(seq);

	OwnedItems owned_items;
	owned_items.raw.reserve(static_cast<size_t>(n));

	std::vector<std::pair<int, int>> dims;
	dims.reserve(static_cast<size_t>(n));

	for (Py_ssize_t i = 0; i < n; ++i) {
		PyObject* pair_obj = seq_items[i];
		PyObject* pair_seq = PySequence_Fast(pair_obj, "each item must be a tuple/list of length 2");
		if (pair_seq == nullptr) {
			Py_DECREF(seq);
			return nullptr;
		}
		if (PySequence_Fast_GET_SIZE(pair_seq) != 2) {
			Py_DECREF(pair_seq);
			Py_DECREF(seq);
			PyErr_SetString(PyExc_ValueError, "each item must contain exactly 2 integers: (width, height)");
			return nullptr;
		}
		PyObject** pair_items = PySequence_Fast_ITEMS(pair_seq);
		const long width = PyLong_AsLong(pair_items[0]);
		if (PyErr_Occurred()) {
			Py_DECREF(pair_seq);
			Py_DECREF(seq);
			return nullptr;
		}
		const long height = PyLong_AsLong(pair_items[1]);
		if (PyErr_Occurred()) {
			Py_DECREF(pair_seq);
			Py_DECREF(seq);
			return nullptr;
		}
		Py_DECREF(pair_seq);
		if (width <= 0 || height <= 0) {
			Py_DECREF(seq);
			PyErr_SetString(PyExc_ValueError, "item width/height must be positive integers");
			return nullptr;
		}
		owned_items.raw.push_back(new StripPacking::item(static_cast<int>(i), static_cast<int>(width), static_cast<int>(height), static_cast<int>(i)));
		dims.emplace_back(static_cast<int>(width), static_cast<int>(height));
	}
	Py_DECREF(seq);

	PyObject* placements = PyDict_New();
	if (placements == nullptr) {
		return nullptr;
	}

	PyObject* state_obj = nullptr;
	PyObject* height_obj = nullptr;
	try {
		if (timeout == 0) {
			state_obj = PyUnicode_FromString("timeout");
			height_obj = Py_None;
			Py_INCREF(Py_None);
		} else {
			std::vector<const StripPacking::item*> const_items;
			const_items.reserve(owned_items.raw.size());
			for (const auto* it : owned_items.raw) {
				const_items.push_back(it);
			}

			StripPacking::BLEU::algStatus = StripPacking::algorithmStatus::exact;
			StripPacking::BLEU solver(const_items, bin_width, timeout);
			solver.setSearchStrategy(branch_and_bound != 0, benders != 0);
			const int height = solver.takeOff();
			if (height >= 0) {
				state_obj = PyUnicode_FromString("completed");
				height_obj = PyLong_FromLong(height);
				const auto placed = solver.getPlacedCoordinates();
				for (const auto& kv : placed) {
					PyObject* key = PyLong_FromLong(kv.first);
					PyObject* value = Py_BuildValue("(ii)", kv.second.x, kv.second.y);
					if (key == nullptr || value == nullptr) {
						Py_XDECREF(key);
						Py_XDECREF(value);
						throw std::runtime_error("Failed to build placement dictionary");
					}
					if (PyDict_SetItem(placements, key, value) < 0) {
						Py_DECREF(key);
						Py_DECREF(value);
						throw std::runtime_error("Failed to set placement dictionary entry");
					}
					Py_DECREF(key);
					Py_DECREF(value);
				}
				if (placed.size() != static_cast<size_t>(n)) {
					std::ostringstream oss;
					oss << "Solver returned an incomplete placement map: got " << placed.size()
						<< " placements for " << n << " items";
					throw std::runtime_error(oss.str());
				}
			} else {
				state_obj = PyUnicode_FromString("timeout");
				height_obj = Py_None;
				Py_INCREF(Py_None);
			}
		}
	} catch (const std::exception& ex) {
		Py_DECREF(placements);
		Py_XDECREF(state_obj);
		Py_XDECREF(height_obj);
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return nullptr;
	}

	if (state_obj == nullptr || height_obj == nullptr) {
		Py_DECREF(placements);
		Py_XDECREF(state_obj);
		Py_XDECREF(height_obj);
		PyErr_SetString(PyExc_RuntimeError, "Internal error building pack_native output");
		return nullptr;
	}

	PyObject* result = Py_BuildValue("{s:O,s:O,s:O}", "state", state_obj, "height", height_obj, "placements", placements);
	Py_DECREF(state_obj);
	Py_DECREF(height_obj);
	Py_DECREF(placements);
	return result;
}

PyMethodDef module_methods[] = {
	{"pack_native", reinterpret_cast<PyCFunction>(pack_native), METH_VARARGS | METH_KEYWORDS, "Run strip packing solver."},
	{nullptr, nullptr, 0, nullptr}
};

struct PyModuleDef module_def = {
	PyModuleDef_HEAD_INIT,
	"_spp_native",
	"Native strip packing extension",
	-1,
	module_methods,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

} // namespace

PyMODINIT_FUNC PyInit__spp_native(void) {
	return PyModule_Create(&module_def);
}
