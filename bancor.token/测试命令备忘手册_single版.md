# bancor发行代币合约测试执行手册(EOS测试链)
## 1. tic用户登陆47.96.237.226节点
    
## 2. 创建测试账户,赋singlebancor@eosio.code权限

```
cleos system newaccount  --transfer eosio eostest11111 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV --stake-net "100.0000 EOS" --stake-cpu "100.0000 EOS" --buy-ram "100.0000 EOS"

cleos system newaccount  --transfer eosio eospayer1111 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV --stake-net "100.0000 EOS" --stake-cpu "100.0000 EOS" --buy-ram "100.0000 EOS"
```


```
cleos set account permission singlebancor active '{"threshold": 1,"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"accounts": [{"permission":{"actor":"singlebancor","permission":"eosio.code"},"weight":1}]}' owner -p singlebancor

cleos set account permission eostest11111 active '{"threshold": 1,"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"accounts": [{"permission":{"actor":"singlebancor","permission":"eosio.code"},"weight":1}]}' owner -p eostest11111

cleos set account permission eospayer1111 active '{"threshold": 1,"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"accounts": [{"permission":{"actor":"singlebancor","permission":"eosio.code"},"weight":1}]}' owner -p eospayer1111

```


```
cleos transfer  eosio eostest11111  "10000000.0000 EOS"
cleos transfer  eosio eospayer1111  "10000000.0000 EOS"


```


## 3.创建代币

```
cleos push action eosbasetoken create '["eosbasetoken", "10000000000.0000 ETHF"]' -p eosbasetoken
cleos push action eosbasetoken issue '[ "eospayer1111", "100.0000 ETHF", "issue" ]' -p eosbasetoken

cleos get currency balance eosbasetoken eospayer1111

/*注册白名单*/
cleos push action singlebancor regadmwhite '["eospayer1111","eospayer1111"]' -p singlebancor

cleos push action singlebancor regwhitelist '["eospayer1111","eospayer1111",true]' -p eospayer1111

cleos push action singlebancor create '["10000000000.0000 OMO","0.0000 OMO",500,"eosbasetoken","10000.0000 ETHF","0.0000 ETHF","10000.0000 OMO","0.0000 OMO","eospayer1111"]' -p eospayer1111

```

## 4.验证创建代币操作

```
cleos get table singlebancor singlebancor banmarket
cleos get table singlebancor singlebancor whitelist
cleos get currency balance eosbasetoken eostest11111

cleos get currency balance eosbasetoken singlebancor

cleos get currency balance singlebancor singlebancor

```

## 5.购买OMO币

```
/*
储备金兑换智能代币估算:兑换数量/储备金总额*智能代币余额
*/
cleos push action singlebancor buy '["eostest11111","100.0000 ETHF","OKO","memo"]' -p eostest11111

```

## 6.验证
```
cleos get currency balance eosbasetoken singlebancor
cleos get currency balance eosbasetoken eostest11111

cleos get currency balance singlebancor singlebancor
cleos get currency balance singlebancor eostest11111


```

## 7.出售OMO币

```
/*
智能代币兑换储备金估算:兑换数量/智能代币余额*储备金总额
*/
cleos push action singlebancor sell '["eostest11111","2.0000 OMO","memo"]' -p eostest11111

```

## 8.验证
```
cleos get currency balance eosbasetoken singlebancor

cleos get currency balance singlebancor singlebancor

cleos get currency balance eosbasetoken eostest11111

cleos get currency balance singlebancor eostest11111

```


## 9.增发代币

```
cleos push action singlebancor issue '["2.0000 OMO","memo"]' -p singlebancor

```

## 10.验证
```
cleos get table singlebancor singlebancor banmarket

cleos get currency balance singlebancor singlebancor

cleos get currency stats singlebancor OMO

```

## 9.回收代币

```
cleos push action singlebancor retire '["2.4999 OMO","memo"]' -p singlebancor

```

## 10.验证
```
cleos get table singlebancor singlebancor banmarket

cleos get currency balance singlebancor singlebancor

cleos get currency stats singlebancor OMO

```

## 9.销毁代币

```
cleos push action eostokenbone destroy '["eospayer1111","OMO","memo"]' -p eostokenbone

```

## 10.验证
```
cleos get currency balance eosbasetoken singlebancor
cleos get table singlebancor singlebancor banmarket

cleos get currency balance singlebancor singlebancor

cleos get currency stats singlebancor OMO

```