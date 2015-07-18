README
======

This is the README for the isomodel (monthly and hourly) building energy model project. 

### How do I get set up? ###

See IsoModel/COMPILING.txt for details on how to compile the code.

#### Compiling the docs ####

The documentation uses doxygen, which can be downloaded from
<http://www.stack.nl/~dimitri/doxygen/download.html>. The doxygen settings are
stored in the Doxyfile in the root of the repo. The docs can by compiled by
running doxygen in the root directory. This will create a directory called
"doc" which will have the docs in it.

Monthly vs Hourly
-----------------

Notes on the current differences between the monthly and hourly simulations. In
general, the monthly simulation is considered to be "correct" and the goal is
to bring the hourly simulation in line with its results, but this is not always
the case. The goal is for the monthly and hourly simulations to produce
relatively consistent results, except in cases where the hourly model is better
able to express the property being studied (e.g., hourly occupancy schedules,
thermal mass, etc.).

Monthly vs hourly values updated 2015-07-16 with IsoModel/test\_data/SmallOffice\_v2.ism

### ElecHeat ###

**Untested** - Electric heating is untested, but presumably consistent because its values
shouldn't differ from gas heating.

| Month | Monthly ElecHeat      | Hourly ElecHeat      | Difference    |
|-------|-----------------------|----------------------|---------------|
| 0     | 0                     | 0                    | 0             |
| 1     | 0                     | 0                    | 0             |
| 2     | 0                     | 0                    | 0             |
| 3     | 0                     | 0                    | 0             |
| 4     | 0                     | 0                    | 0             |
| 5     | 0                     | 0                    | 0             |
| 6     | 0                     | 0                    | 0             |
| 7     | 0                     | 0                    | 0             |
| 8     | 0                     | 0                    | 0             |
| 9     | 0                     | 0                    | 0             |
| 10    | 0                     | 0                    | 0             |
| 11    | 0                     | 0                    | 0             |

### ElecCool ###

**Consistent** - Cooling results are fairly consistent across monthly and hourly simulations.

| Month | Monthly ElecCool | Hourly ElecCool | Difference |
|-------|------------------|-----------------|------------|
| 0     | 0.0231166        | 0               | 0.0231166  |
| 1     | 0.045225         | 0               | 0.045225   |
| 2     | 0.135984         | 0               | 0.135984   |
| 3     | 0.341924         | 0.803466        | -0.461542  |
| 4     | 1.08367          | 1.5465          | -0.462828  |
| 5     | 2.12931          | 3.21538         | -1.08608   |
| 6     | 3.28741          | 4.64883         | -1.36142   |
| 7     | 1.79919          | 2.73518         | -0.935989  |
| 8     | 0.758612         | 1.27682         | -0.51821   |
| 9     | 0.180504         | 0.0637051       | 0.116799   |
| 10    | 0.0404312        | 0               | 0.0404312  |
| 11    | 0.0185314        | 0               | 0.0185314  |

### ElecIntLights ###

**Consistent** - After commit #3124f63, monthly and hourly interior lighting results are
consistent.

| Month | Monthly ElecIntLights | Hourly ElecIntLights | Difference |
|-------|-----------------------|----------------------|------------|
| 0     | 2.721                 | 2.74978              | -0.0287845 |
| 1     | 2.45768               | 2.48852              | -0.0308406 |
| 2     | 2.721                 | 2.78731              | -0.0663075 |
| 3     | 2.63322               | 2.61266              | 0.0205609  |
| 4     | 2.721                 | 2.78731              | -0.0663075 |
| 5     | 2.63322               | 2.68771              | -0.0544852 |
| 6     | 2.721                 | 2.71226              | 0.00873865 |
| 7     | 2.721                 | 2.78731              | -0.0663075 |
| 8     | 2.63322               | 2.65019              | -0.0169621 |
| 9     | 2.721                 | 2.74978              | -0.0287845 |
| 10    | 2.63322               | 2.68771              | -0.0544853 |
| 11    | 2.721                 | 2.71226              | 0.00873855 |

### ElecExtLights ###

**Consistent** - Exterior lighting is consistent.

| Month | Monthly ElecExtLights | Hourly ElecExtLights | Difference    |
|-------|-----------------------|----------------------|---------------|
| 0     | 0.257822              | 0.257822             | -2.38698e-015 |
| 1     | 0.199604              | 0.207327             | -0.00772279   |
| 2     | 0.184159              | 0.210892             | -0.0267327    |
| 3     | 0.178218              | 0.198416             | -0.0201981    |
| 4     | 0.147327              | 0.159208             | -0.0118812    |
| 5     | 0.142575              | 0.142575             | -2.77556e-017 |
| 6     | 0.147327              | 0.148515             | -0.00118812   |
| 7     | 0.147327              | 0.178218             | -0.0308912    |
| 8     | 0.178218              | 0.189505             | -0.0112872    |
| 9     | 0.202575              | 0.226931             | -0.0243565    |
| 10    | 0.231684              | 0.236436             | -0.00475249   |
| 11    | 0.257822              | 0.258416             | -0.000594061  |

### ElecFans ###

**Consistent** - Since fixing issue #44, fan results are consistent.

| Month | Monthly ElecFans | Hourly ElecFans | Difference |
|-------|------------------|-----------------|------------|
| 0     | 7.4786           | 7.25748         | 0.221118   |
| 1     | 6.05336          | 5.78027         | 0.273083   |
| 2     | 4.72618          | 4.20455         | 0.521632   |
| 3     | 2.71281          | 2.82539         | -0.112577  |
| 4     | 1.40998          | 1.56159         | -0.15161   |
| 5     | 0.837238         | 1.3296          | -0.49236   |
| 6     | 1.13737          | 1.59728         | -0.459908  |
| 7     | 0.688762         | 1.0586          | -0.369836  |
| 8     | 0.875118         | 1.02824         | -0.153118  |
| 9     | 2.76048          | 2.22808         | 0.532401   |
| 10    | 4.68014          | 4.26135         | 0.418792   |
| 11    | 7.17145          | 7.00312         | 0.168325   |

### ElecPump ###

**Inconsistent** - Issue #43. Pump results are not consistent. Both fan and pump values differ the most in
the winter. Their is potentially an error in the monthly pump values.

| Month | Monthly ElecPump | Hourly ElecPump | Difference |
|-------|------------------|-----------------|------------|
| 0     | 0.808168         | 0.186           | 0.622168   |
| 1     | 0.654151         | 0.168           | 0.486151   |
| 2     | 0.510731         | 0.1765          | 0.334231   |
| 3     | 0.293157         | 0.15375         | 0.139407   |
| 4     | 0.152368         | 0.138           | 0.0143682  |
| 5     | 0.0904754        | 0.12175         | -0.0312746 |
| 6     | 0.122909         | 0.119           | 0.00390898 |
| 7     | 0.0744305        | 0.11075         | -0.0363195 |
| 8     | 0.0945689        | 0.11675         | -0.0221811 |
| 9     | 0.298309         | 0.16            | 0.138309   |
| 10    | 0.505756         | 0.17275         | 0.333006   |
| 11    | 0.774976         | 0.186           | 0.588976   |

### ElecEquipInt ###

**Consistent** - Issue #45 fixed.

| Month | Monthly ElecEquipInt | Hourly ElecEquipInt | Difference   |
|-------|----------------------|---------------------|--------------|
| 0     | 2.24457              | 2.24088             | 0.00368978   |
| 1     | 2.02735              | 2.02735             | 1.86517e-014 |
| 2     | 2.24457              | 2.26671             | -0.0221387   |
| 3     | 2.17216              | 2.13526             | 0.0368978    |
| 4     | 2.24457              | 2.26671             | -0.0221387   |
| 5     | 2.17216              | 2.18692             | -0.0147591   |
| 6     | 2.24457              | 2.21505             | 0.0295182    |
| 7     | 2.24457              | 2.26671             | -0.0221387   |
| 8     | 2.17216              | 2.16109             | 0.0110693    |
| 9     | 2.24457              | 2.24088             | 0.00368978   |
| 10    | 2.17216              | 2.18692             | -0.0147591   |
| 11    | 2.24457              | 2.21505             | 0.0295182    |

### ElecEquipExt ###

**Untested** - Exterior equipment defaults to zero at the moment, so it is untested.

| Month | Monthly ElecEquipExt  | Hourly ElecEquipExt  | Difference    |
|-------|-----------------------|----------------------|---------------|
| 0     | 0                     | 0                    | 0             |
| 1     | 0                     | 0                    | 0             |
| 2     | 0                     | 0                    | 0             |
| 3     | 0                     | 0                    | 0             |
| 4     | 0                     | 0                    | 0             |
| 5     | 0                     | 0                    | 0             |
| 6     | 0                     | 0                    | 0             |
| 7     | 0                     | 0                    | 0             |
| 8     | 0                     | 0                    | 0             |
| 9     | 0                     | 0                    | 0             |
| 10    | 0                     | 0                    | 0             |
| 11    | 0                     | 0                    | 0             |

### ElectDHW ###

**Untested** - Electric hot water defaults to zero at the moment, so it is untested.

| Month | Monthly ElectDHW      | Hourly ElectDHW      | Difference    |
|-------|-----------------------|----------------------|---------------|
| 0     | 0                     | 0                    | 0             |
| 1     | 0                     | 0                    | 0             |
| 2     | 0                     | 0                    | 0             |
| 3     | 0                     | 0                    | 0             |
| 4     | 0                     | 0                    | 0             |
| 5     | 0                     | 0                    | 0             |
| 6     | 0                     | 0                    | 0             |
| 7     | 0                     | 0                    | 0             |
| 8     | 0                     | 0                    | 0             |
| 9     | 0                     | 0                    | 0             |
| 10    | 0                     | 0                    | 0             |
| 11    | 0                     | 0                    | 0             |

### GasHeat ###

**Consistent** - Gas heat is consistent.

| Month | Monthly GasHeat | Hourly GasHeat | Difference |
|-------|-----------------|----------------|------------|
| 0     | 42.5212         | 41.3248        | 1.19643    |
| 1     | 34.3655         | 32.9002        | 1.46528    |
| 2     | 26.6327         | 23.8715        | 2.76125    |
| 3     | 14.7675         | 14.4764        | 0.291096   |
| 4     | 5.89133         | 5.79526        | 0.0960709  |
| 5     | 0.572288        | 1.42311        | -0.850818  |
| 6     | 0               | 0.280847       | -0.280847  |
| 7     | 0.377265        | 0.765168       | -0.387902  |
| 8     | 3.48712         | 3.24793        | 0.239184   |
| 9     | 15.3567         | 12.406         | 2.9507     |
| 10    | 26.5589         | 24.208         | 2.3509     |
| 11    | 40.782          | 39.8604        | 0.92158    |

### GasCool ###

**Untested** - Gas cooling is untested, but presumably equally consistant to electric cooling,
as the fuel source shouldn't change the calculations.

| Month | Monthly GasCool       | Hourly GasCool       | Difference    |
|-------|-----------------------|----------------------|---------------|
| 0     | 0                     | 0                    | 0             |
| 1     | 0                     | 0                    | 0             |
| 2     | 0                     | 0                    | 0             |
| 3     | 0                     | 0                    | 0             |
| 4     | 0                     | 0                    | 0             |
| 5     | 0                     | 0                    | 0             |
| 6     | 0                     | 0                    | 0             |
| 7     | 0                     | 0                    | 0             |
| 8     | 0                     | 0                    | 0             |
| 9     | 0                     | 0                    | 0             |
| 10    | 0                     | 0                    | 0             |
| 11    | 0                     | 0                    | 0             |

### GasEquip ###

**Untested** - Gas equipment is untested.

| Month | Monthly GasEquip      | Hourly GasEquip      | Difference    |
|-------|-----------------------|----------------------|---------------|
| 0     | 0                     | 0                    | 0             |
| 1     | 0                     | 0                    | 0             |
| 2     | 0                     | 0                    | 0             |
| 3     | 0                     | 0                    | 0             |
| 4     | 0                     | 0                    | 0             |
| 5     | 0                     | 0                    | 0             |
| 6     | 0                     | 0                    | 0             |
| 7     | 0                     | 0                    | 0             |
| 8     | 0                     | 0                    | 0             |
| 9     | 0                     | 0                    | 0             |
| 10    | 0                     | 0                    | 0             |
| 11    | 0                     | 0                    | 0             |

### GasDHW ###

**Untested** - Gas hot water defaults to zero, so is currently untested.

| Month | Monthly GasDHW        | Hourly GasDHW        | Difference    |
|-------|-----------------------|----------------------|---------------|
| 0     | 0                     | 0                    | 0             |
| 1     | 0                     | 0                    | 0             |
| 2     | 0                     | 0                    | 0             |
| 3     | 0                     | 0                    | 0             |
| 4     | 0                     | 0                    | 0             |
| 5     | 0                     | 0                    | 0             |
| 6     | 0                     | 0                    | 0             |
| 7     | 0                     | 0                    | 0             |
| 8     | 0                     | 0                    | 0             |
| 9     | 0                     | 0                    | 0             |
| 10    | 0                     | 0                    | 0             |
| 11    | 0                     | 0                    | 0             |
