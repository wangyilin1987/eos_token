/**
 *  @file 锁仓
 *  @copyright wyl
 */

#include "token.timelock.hpp"

void TokenTimeLock::token_lock(account_name token_contract, account_name locker, asset lock_quantity, uint64_t release_time)
{    
    print("|token_lock begin...***");
    /*登记用户锁仓表*/
    locks locktable(_self,_self);
    auto lockiter = locktable.find( locker );
    if( lockiter == locktable.end() )
    {        
        locktable.emplace( _self, [&]( auto& exa )
        {
            exa.locker = locker;
            exa.token_contract = token_contract;
            exa.quantity = lock_quantity;
            exa.release_time = release_time;
        });
        print("|emplace end...***");
    } 
    else 
    {
        /*该用户已被锁仓*/
        eosio_assert( false, "user is locked.." );
    }
}

void TokenTimeLock::token_unlock(account_name unlocker, struct lockinfst *ret)
{    
    uint64_t time_now = now();
    
    print("|time_now:",time_now,"|unlock begin...");
    locks locktable(_self,_self);
    auto lockiter = locktable.find( unlocker );
    if( lockiter == locktable.end() ) 
    {
        eosio_assert( false, "No locking required.." );
    } 
    else 
    {
        print("|lockiter->release_time:",lockiter->release_time);
        eosio_assert( time_now >= lockiter->release_time, "not yet time to unlock..." );        
        
        ret->token_contract = lockiter->token_contract;
        ret->quantity = lockiter->quantity;
        
        print("token_contract:",ret->token_contract,"|***");
        ret->quantity.print();
         
        /*删除锁仓记录*/
        locktable.erase(lockiter);
        
        return ;
    }
}
