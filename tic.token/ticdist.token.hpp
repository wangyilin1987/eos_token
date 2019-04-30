/**
 *  @file 代币控制合约
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosio.token.hpp>

#include <string>

#include "token.timelock.hpp"


using namespace eosio;
using std::string;

class TicDistToken : public contract {
    public:
    /**
      * FUNC: 构造函数
      * INPUT:
      *      self: 合约拥有者账户
      * OUTPUT:                        
      * RETURN: 
      * DESC:                            
    */
    TicDistToken( account_name self ):contract(self),_locks(self){}


    /**
      * FUNC: 初始发行代币函数
      * INPUT:                           
      *      issue: 发行方A
      *      quantity：初始发行给发行方A的Token数
      *      lock_flag: 锁仓标志(0:不锁仓；1:锁仓)
      *      lock_rate：锁仓比例(eg:50表示锁仓50%)
      *      lock_time: 锁仓时间（单位:天）
      *      memo: 备注
      * OUTPUT:
      * RETURN: 
      * DESC:                            
    */
    void issue(account_name issue, 
                asset quantity,       
                uint64_t lock_flag, 
                uint64_t lock_rate, 
                uint64_t lock_time, 
                string memo );

    /**
      * FUNC: 增发代币
      * INPUT:
      *      addissue: 增发代币接收方
      *      token: 增发的Token
      *      memo: 备注
      * OUTPUT:
      * RETURN:  
      * DESC:
    */
    void addissue( account_name addissue, asset token, string memo );
    
    /**
      * FUNC: 锁仓解锁
      * INPUT:
      *      locker: 解锁账户
      *      memo: 备注
      * OUTPUT:
      * RETURN:
      * DESC:
    */
    void unlock(account_name unlocker, string memo);
     
    /**
      * FUNC: 发行参数设置
      * INPUT:
      *      token_contract:代币合约  
      *      token:发行的Token
      *      issuernum: 初始发行方数量
      *      dividend_rate: 通胀率(天)的被除数
      *      divisor_rate: 通胀率(天)的除数
      * OUTPUT:
      * RETURN:  
      * DESC:
    */
    void setconfig(account_name token_contract, asset token, uint64_t issuernum, uint64_t dividend_rate, uint64_t divisor_rate);

    /**
      * FUNC: 代币销毁
      * INPUT:
      *      quantity:销毁的代币数量
      *      memo: 备注
      * OUTPUT:
      * RETURN:  
      * DESC:把销毁的代币transfer到代币合约
    */
    void destroy( asset quantity, string memo  );
     
    private:

    TokenTimeLock _locks;

    /*参数控制表*/
    ///@abi table issuernumtb i64
    struct issnum {
        account_name      token_contract;/*代币合约*/
        asset             max_quantity;  /*累计发行的Token数*/
        uint64_t          iss_count;     /*调用过issue接口的次数*/
        uint64_t          max_num;       /*允许调用issue接口的最大次数*/

        uint64_t          dividend_rate; /*通胀率(天)的被除数*/
        uint64_t          divisor_rate;  /*通胀率(天)的除数*/
        asset             quantity;      /*最近一次增发的Token数*/
        uint64_t          add_count;     /*add_issue接口调用次数*/
        time_t            add_time;      /*最近一次调用时间*/
        
        asset             destroy_quantity; /*销毁的Token数*/
        asset             lock_quantity;    /*锁仓的Token数*/

        uint64_t primary_key()const { return quantity.symbol.name(); }
    };
    
    typedef eosio::multi_index<N(issuernumtb), issnum> issuernumst;
};

