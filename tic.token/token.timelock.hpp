/**
 *  @file 锁仓
 *  @copyright wyl
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

#include <string>
using namespace eosio;
using std::string;

/*作为接口返回值*/ 
struct lockinfst
{
    account_name      token_contract;
    asset             quantity;
    
    EOSLIB_SERIALIZE( lockinfst, (token_contract)(quantity) )
};

class TokenTimeLock : public contract {
  public:
    /**
      * FUNC: 构造函数
      * INPUT:                             
      *      self: 合约拥有者账户
      * OUTPUT:                        
      * RETURN: 
      * DESC:                            
    */
    TokenTimeLock( account_name self ):contract(self){}

    /**
      * FUNC: 登记锁仓
      * INPUT:
      *      token_contract:Token合约
      *      locker: 锁仓用户
      *      lock_quantity: 锁仓Token数
      *      releaseTime：解锁时间(1970年到解锁时间时的秒数)
      * OUTPUT:                        
      * RETURN: 
      * DESC:
    */
    void token_lock(account_name token_contract, account_name locker, asset lock_quantity, uint64_t release_time);

    /**
      * FUNC: 解锁
      * INPUT:
            locker: 锁仓用户
      * OUTPUT:
      *     ret: 返回信息:锁仓的Token合约和锁仓的Token数
      * RETURN: 
      * DESC:
    */
    void token_unlock(account_name unlocker, struct lockinfst *ret);

  private:

    /*锁仓表*/
    ///@abi table locktb i64
    struct lockst 
    {
        account_name      locker;           /*锁仓用户*/
        account_name      token_contract;   /*代币合约*/
        asset             quantity;         /*锁仓Token数量*/
        uint64_t          release_time;     /*解锁时间*/

        uint64_t primary_key()const { return locker; }
    };

    typedef eosio::multi_index<N(locktb), lockst> locks;
};


;

