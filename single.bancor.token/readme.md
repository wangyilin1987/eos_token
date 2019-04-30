```
cleos system newaccount  --transfer eosio singlebancor EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV --stake-net "100.0000 EOS" --stake-cpu "100.0000 EOS" --buy-ram "100.0000 EOS"

```
cleos set account permission singlebancor active '{"threshold": 1,"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"accounts": [{"permission":{"actor":"singlebancor","permission":"eosio.code"},"weight":1}]}' owner -p singlebancor

cleos set account permission eospayer1111 active '{"threshold": 1,"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"accounts": [{"permission":{"actor":"singlebancor","permission":"eosio.code"},"weight":1}]}' owner -p eospayer1111

```
```

```
eosiocpp -g single.bancor.token.abi single.eosio.token_bancor.cpp 
eosiocpp -o single.bancor.token.wast single.eosio.token_bancor.cpp 

cleos set contract singlebancor ../single.bancor.token/ -p singlebancor

```

```

cleos push action singlebancor regadmwhite '["eospayer1111","eospayer1111"]' -p singlebancor
cleos push action singlebancor regwhitelist '["eospayer1111","eospayer1111",true]' -p eospayer1111

cleos push action singlebancor create '["1000000000.0000 SINOMO","0.0000 SINOMO",1000,"eosio.token","50000.0000 EOS","10000.0000 EOS","40000.0000 SINOMO","10000.0000 SINOMO","eospayer1111"]' -p eospayer1111 

cleos get table singlebancor singlebancor banmarket
```

```
cleos push action eosio.token transfer '[ "eostest11111", "singlebancor", "3.0000 EOS","SINOMO" ]' -p eostest11111

cleos get currency balance singlebancor eostest11111
cleos get currency balance singlebancor eospayer1111

```

```
cleos push action singlebancor transfer '[ "eostest11111", "singlebancor", "10.0000 SINOMO","EOS" ]' -p eostest11111


```

```
cleos get currency balance singlebancor eostest11111

cleos push action singlebancor transfer '[ "eostest11111", "singlebancor", "0.9999 SINOMO" ]' -p eostest11111
cleos push action singlebancor transfer '[ "eospayer1111", "singlebancor", "50000.0000 SINOMO" ]' -p eospayer1111

cleos push action singlebancor destroy '["eospayer1111","SINOMO","memo"]' -p eospayer1111
```