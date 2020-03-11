\ENCODING=ISO-8859-1
\Problem name: IloCplex

Minimize
 obj: Var_0#0 + Var_1#1 + Var_2#2 + Var_3#3 + Var_4#4 + Var_5#5 + Var_6#6
      + Var_7#7 + Var_8#8 + Var_9#9 + Var_10#10 + Var_11#11 + Var_12#12
      + Var_13#13 + Var_14#14 + Var_15#15 + Var_16#16 + Var_17#17 + Var_18#18
      + Var_19#19 + Var_20#20 + Var_21#21 + Var_22#22 + Var_23#23 + Var_24#24
      + Var_25#25 + Var_26#26 + Var_27#27 + Var_28#28 + Var_29#29 + Var_30#30
      + Var_31#31 + Var_32#32 + Var_33#33 + Var_34#34 + Var_35#35 + Var_36#36
      + Var_37#37 + Var_38#38 + Var_39#39 + Var_40#40 + Var_41#41 + Var_42#42
      + Var_43#43 + Var_44#44 + Var_45#45 + Var_46#46 + Var_47#47 + Var_48#48
      + Var_49#49 + Var_50#50 + Var_51#51 + Var_52#52 + Var_53#53 + Var_54#54
      + Var_55#55 + Var_56#56 + Var_57#57 + Var_58#58 + Var_59#59 + Var_60#60
      + Var_61#61 + Var_62#62 + Var_63#63 + Var_64#64 + Var_65#65 + Var_66#66
      + Var_67#67 + Var_68#68 + Var_69#69 + Var_70#70 + Var_71#71
Subject To
 Cstr_0#0:   Var_0#0 + Var_37#37 + Var_39#39 + Var_40#40 + Var_42#42
             + Var_59#59 >= 7
 Cstr_1#1:   Var_1#1 + Var_36#36 + Var_39#39 + Var_50#50 + Var_52#52
             + Var_58#58 + Var_65#65 >= 6
 Cstr_2#2:   Var_2#2 + Var_33#33 + Var_38#38 + Var_43#43 + Var_57#57
             + Var_66#66 >= 4
 Cstr_3#3:   Var_3#3 + Var_32#32 + Var_34#34 + Var_43#43 + Var_44#44
             + Var_51#51 + Var_53#53 + Var_55#55 + Var_56#56 + Var_61#61
             + Var_64#64 + Var_65#65 + Var_67#67 >= 5
 Cstr_4#4:   Var_4#4 + Var_33#33 + Var_34#34 + Var_35#35 + Var_36#36
             + Var_41#41 + Var_42#42 + Var_54#54 + Var_60#60 + Var_70#70 >= 10
 Cstr_5#5:   Var_5#5 + Var_31#31 + Var_35#35 + Var_40#40 + Var_46#46
             + Var_47#47 + Var_63#63 >= 6
 Cstr_6#6:   Var_6#6 + Var_31#31 + Var_32#32 + Var_37#37 + Var_41#41
             + Var_46#46 + Var_55#55 + Var_62#62 + Var_68#68 + Var_69#69
             + Var_70#70 + Var_71#71 >= 9
 Cstr_7#7:   Var_7#7 + Var_31#31 + Var_33#33 + Var_44#44 + Var_45#45
             + Var_49#49 + Var_51#51 >= 7
 Cstr_8#8:   Var_8#8 + Var_30#30 + Var_32#32 + Var_36#36 + Var_40#40
             + Var_42#42 + Var_49#49 + Var_57#57 >= 7
 Cstr_9#9:   Var_9#9 + Var_30#30 + Var_44#44 + Var_47#47 + Var_50#50
             + Var_52#52 + Var_56#56 + Var_71#71 >= 2
 Cstr_10#10: Var_10#10 + Var_30#30 + Var_34#34 + Var_38#38 + Var_45#45
             + Var_46#46 + Var_48#48 + Var_52#52 + Var_62#62 + Var_63#63
             + Var_69#69 >= 6
 Cstr_11#11: Var_11#11 + Var_30#30 + Var_35#35 + Var_37#37 + Var_41#41
             + Var_43#43 + Var_48#48 + Var_50#50 + Var_51#51 + Var_53#53
             + Var_56#56 + Var_60#60 + Var_61#61 + Var_63#63 + Var_66#66
             + Var_67#67 + Var_68#68 + Var_71#71 >= 6
 Cstr_12#12: Var_12#12 + Var_32#32 + Var_35#35 + Var_38#38 + Var_45#45
             + Var_48#48 + Var_53#53 + Var_54#54 + Var_56#56 + Var_58#58
             + Var_61#61 + Var_68#68 >= 4
 Cstr_13#13: Var_13#13 + Var_30#30 + Var_31#31 + Var_38#38 + Var_41#41
             + Var_45#45 + Var_47#47 + Var_49#49 + Var_51#51 + Var_59#59
             + Var_63#63 + Var_64#64 + Var_67#67 + Var_69#69 + Var_70#70
             + Var_71#71 >= 9
 Cstr_14#14: Var_14#14 + Var_29#29 + Var_34#34 + Var_37#37 + Var_45#45
             + Var_47#47 + Var_48#48 + Var_49#49 + Var_54#54 + Var_55#55
             + Var_57#57 + Var_58#58 + Var_60#60 + Var_61#61 + Var_62#62
             + Var_63#63 + Var_65#65 + Var_66#66 + Var_69#69 + Var_70#70 >= 10
 Cstr_15#15: Var_15#15 + Var_29#29 + Var_38#38 + Var_44#44 + Var_48#48
             + Var_49#49 + Var_50#50 + Var_54#54 + Var_57#57 + Var_59#59
             + Var_60#60 + Var_62#62 + Var_64#64 + Var_65#65 + Var_68#68 >= 7
 Cstr_16#16: Var_16#16 + Var_29#29 + Var_58#58 + Var_60#60 + Var_62#62 >= 2
 Cstr_17#17: Var_17#17 + Var_30#30 + Var_39#39 + Var_53#53 + Var_54#54 >= 3
 Cstr_18#18: Var_18#18 + Var_29#29 + Var_33#33 + Var_39#39 + Var_46#46
             + Var_47#47 + Var_48#48 + Var_49#49 + Var_51#51 + Var_57#57
             + Var_58#58 + Var_61#61 + Var_64#64 + Var_67#67 + Var_68#68
             + Var_69#69 + Var_70#70 + Var_71#71 >= 11
 Cstr_19#19: Var_19#19 + Var_29#29 + Var_36#36 + Var_44#44 + Var_46#46
             + Var_47#47 + Var_53#53 + Var_55#55 + Var_59#59 + Var_60#60
             + Var_66#66 + Var_68#68 >= 8
 Cstr_20#20: Var_20#20 + Var_29#29 + Var_40#40 + Var_41#41 + Var_45#45
             + Var_48#48 + Var_52#52 + Var_54#54 + Var_55#55 + Var_62#62
             + Var_64#64 + Var_66#66 + Var_67#67 + Var_68#68 + Var_71#71 >= 7
 Cstr_21#21: Var_21#21 + Var_29#29 + Var_43#43 + Var_45#45 + Var_46#46
             + Var_49#49 + Var_50#50 + Var_54#54 + Var_59#59 + Var_62#62
             + Var_67#67 >= 6
 Cstr_22#22: Var_22#22 + Var_29#29 + Var_56#56 + Var_57#57 + Var_59#59
             + Var_60#60 + Var_64#64 + Var_69#69 >= 3
 Cstr_23#23: Var_23#23 + Var_29#29 + Var_31#31 + Var_42#42 + Var_47#47
             + Var_48#48 + Var_52#52 + Var_53#53 + Var_55#55 + Var_58#58
             + Var_64#64 + Var_66#66 + Var_67#67 + Var_69#69 >= 11
 Cstr_24#24: Var_24#24 + Var_29#29 + Var_61#61 + Var_64#64 + Var_66#66
             + Var_70#70 >= 1
 Cstr_25#25: Var_25#25 + Var_29#29 + Var_49#49 + Var_52#52 + Var_53#53
             + Var_56#56 + Var_58#58 + Var_59#59 + Var_63#63 + Var_66#66
             + Var_69#69 + Var_71#71 >= 4
 Cstr_26#26: Var_26#26 + Var_29#29 + Var_50#50 + Var_53#53 + Var_55#55
             + Var_61#61 + Var_63#63 >= 4
 Cstr_27#27: Var_27#27 + Var_29#29 + Var_51#51 + Var_52#52 + Var_54#54
             + Var_57#57 + Var_58#58 + Var_59#59 + Var_62#62 + Var_63#63
             + Var_65#65 + Var_68#68 + Var_71#71 >= 4
 Cstr_28#28: Var_28#28 + Var_29#29 + Var_59#59 + Var_61#61 + Var_62#62
             + Var_67#67 + Var_68#68 + Var_69#69 >= 2
End
