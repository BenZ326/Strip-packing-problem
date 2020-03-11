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
      + Var_61#61 + Var_62#62 + Var_63#63 + Var_64#64
Subject To
 Cstr_0#0:   Var_0#0 + Var_27#27 + Var_29#29 + Var_30#30 + Var_57#57 >= 9
 Cstr_1#1:   Var_1#1 + Var_28#28 + Var_31#31 + Var_53#53 + Var_62#62 >= 9
 Cstr_2#2:   Var_2#2 + Var_32#32 + Var_34#34 + Var_36#36 + Var_37#37
             + Var_56#56 >= 9
 Cstr_3#3:   Var_3#3 + Var_25#25 + Var_35#35 + Var_38#38 + Var_42#42
             + Var_46#46 + Var_51#51 + Var_52#52 + Var_59#59 >= 9
 Cstr_4#4:   Var_4#4 + Var_26#26 + Var_29#29 + Var_35#35 + Var_40#40
             + Var_51#51 + Var_64#64 >= 9
 Cstr_5#5:   Var_5#5 + Var_26#26 + Var_33#33 + Var_38#38 + Var_39#39
             + Var_48#48 + Var_54#54 + Var_59#59 >= 9
 Cstr_6#6:   Var_6#6 + Var_26#26 + Var_27#27 + Var_28#28 + Var_41#41
             + Var_50#50 + Var_52#52 + Var_61#61 + Var_63#63 >= 21
 Cstr_7#7:   Var_7#7 + Var_24#24 + Var_35#35 + Var_39#39 + Var_44#44
             + Var_45#45 + Var_50#50 + Var_54#54 + Var_60#60 >= 10
 Cstr_8#8:   Var_8#8 + Var_24#24 + Var_25#25 + Var_27#27 + Var_31#31
             + Var_33#33 + Var_36#36 + Var_37#37 + Var_40#40 + Var_43#43
             + Var_45#45 + Var_49#49 + Var_50#50 + Var_55#55 + Var_58#58
             + Var_60#60 >= 31
 Cstr_9#9:   Var_9#9 + Var_24#24 + Var_33#33 + Var_36#36 + Var_38#38
             + Var_41#41 + Var_42#42 + Var_45#45 + Var_47#47 + Var_56#56
             + Var_57#57 + Var_58#58 + Var_60#60 + Var_61#61 + Var_62#62 >= 11
 Cstr_10#10: Var_10#10 + Var_24#24 + Var_34#34 + Var_41#41 + Var_44#44
             + Var_46#46 + Var_47#47 + Var_49#49 + Var_53#53 + Var_55#55
             + Var_58#58 >= 11
 Cstr_11#11: Var_11#11 + Var_25#25 + Var_32#32 + Var_43#43 + Var_44#44
             + Var_47#47 + Var_49#49 + Var_61#61 >= 11
 Cstr_12#12: Var_12#12 + Var_23#23 + Var_30#30 + Var_33#33 + Var_35#35
             + Var_41#41 + Var_43#43 + Var_48#48 + Var_49#49 + Var_54#54
             + Var_63#63 + Var_64#64 >= 14
 Cstr_13#13: Var_13#13 + Var_23#23 + Var_31#31 + Var_42#42 + Var_44#44
             + Var_49#49 + Var_54#54 + Var_56#56 + Var_63#63 + Var_64#64 >= 14
 Cstr_14#14: Var_14#14 + Var_23#23 + Var_32#32 + Var_45#45 + Var_46#46
             + Var_47#47 + Var_48#48 + Var_49#49 + Var_52#52 + Var_55#55
             + Var_58#58 + Var_59#59 + Var_61#61 + Var_63#63 + Var_64#64 >= 14
 Cstr_15#15: Var_15#15 + Var_23#23 + Var_36#36 + Var_39#39 + Var_40#40
             + Var_42#42 + Var_43#43 + Var_46#46 + Var_47#47 + Var_54#54
             + Var_55#55 + Var_58#58 + Var_62#62 + Var_63#63 >= 12
 Cstr_16#16: Var_16#16 + Var_23#23 + Var_37#37 + Var_47#47 + Var_48#48
             + Var_51#51 + Var_54#54 + Var_60#60 + Var_63#63 >= 12
 Cstr_17#17: Var_17#17 + Var_23#23 + Var_38#38 + Var_40#40 + Var_44#44
             + Var_52#52 + Var_53#53 + Var_57#57 + Var_58#58 + Var_60#60
             + Var_63#63 + Var_64#64 >= 12
 Cstr_18#18: Var_18#18 + Var_23#23 + Var_25#25 + Var_28#28 + Var_33#33
             + Var_34#34 + Var_37#37 + Var_39#39 + Var_40#40 + Var_42#42
             + Var_43#43 + Var_44#44 + Var_45#45 + Var_46#46 + Var_48#48
             + Var_51#51 + Var_52#52 + Var_53#53 + Var_55#55 + Var_56#56
             + Var_57#57 + Var_59#59 + Var_61#61 + Var_64#64 >= 30
 Cstr_19#19: Var_19#19 + Var_23#23 + Var_24#24 + Var_29#29 + Var_31#31
             + Var_32#32 + Var_37#37 + Var_39#39 + Var_41#41 + Var_43#43
             + Var_44#44 + Var_45#45 + Var_46#46 + Var_55#55 + Var_59#59
             + Var_60#60 + Var_62#62 >= 30
 Cstr_20#20: Var_20#20 + Var_23#23 + Var_34#34 + Var_43#43 + Var_45#45
             + Var_50#50 + Var_51#51 + Var_58#58 + Var_61#61 + Var_62#62
             + Var_64#64 >= 13
 Cstr_21#21: Var_21#21 + Var_24#24 + Var_30#30 + Var_32#32 + Var_40#40
             + Var_42#42 + Var_43#43 + Var_48#48 + Var_49#49 + Var_50#50
             + Var_53#53 + Var_55#55 + Var_56#56 + Var_60#60 >= 13
 Cstr_22#22: Var_22#22 + Var_25#25 + Var_30#30 + Var_31#31 + Var_34#34
             + Var_39#39 + Var_41#41 + Var_47#47 + Var_48#48 + Var_50#50
             + Var_55#55 + Var_57#57 >= 13
End
