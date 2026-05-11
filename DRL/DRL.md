This is an instruction for the deep reinforcement learning project for the strip packing project.
The core idea is to train a DRL agent to place items given a sequence of items.
The definition of the RL framework:
1. State: a state is defined as the skyline vector;the vector of the remaining items (order matters); the width of the bin;
2. Action: the available coordinates where I can place the next item; the coordinates have to be on the skylines;
3. Reward: -1 * height, where height is the height of the final packing which packs all the items.
