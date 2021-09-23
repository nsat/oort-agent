Support files for generating the adcs hk description

adcs-v3.csv - dump from [LEMADCS_API_3 google doc](https://docs.google.com/spreadsheets/d/19QqpRwgsRerY6U499gjf0-Ir_uRo-is78alt72Zrw1E/edit#gid=0)

lemadcs_adcs_hk.csv - export of the lemadcs_adcs_hk structure from "API
Planner" sheet of the above doc

ml_spec.py - processes the above exports to produce a dsdk definition, oapi
schema definition, or python method to copy all the fields from the spoc
result.
