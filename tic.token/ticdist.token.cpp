/**
 *  @file 代币控制合约
 *  @copyright defined in eos/LICENSE.txt
 */

#include "ticdist.token.hpp"


void TicDistToken::issue( account_name issue, 
                         asset quantity, 
                         uint64_t lock_flag, 
                         uint64_t lock_rate, 
                         uint64_t lock_time, 
                         string memo )
{
    require_auth( _self );
    print("issue test:", quantity.amount,";",lock_flag,";",lock_rate,";",lock_time,";",memo.c_str(),"***");
    quantity.print();
    print("***");

    /*校验锁仓参数*/    
    eosio_assert( lock_flag == 0 || lock_flag == 1, "invalid lock_flag" );
    eosio_assert( lock_rate <= 100, "invalid lock_rate" );
    
    /*控制最高只能锁仓10年*/
    eosio_assert( lock_time <= 3659, "invalid lock_time" );

    /*校验issue调用次数是否超过限制(issuernum)*/
    issuernumst isstable(_self,_self);
    auto issiter = isstable.find( quantity.symbol.name());
    if( issiter == isstable.end() ) 
    {
        eosio_assert( false, "need to call setconfig..." );
    }
    else 
    {        
        print("issiter->iss_count:",issiter->iss_count,"|issiter->max_num:",issiter->max_num);
        
        /*校验issue接口调用次数是否超过限制(issnum.max_num)*/
        eosio_assert( (issiter->iss_count+1) <= issiter->max_num, "call issue time over max" );

        /*累计issue接口调用次数以及初始发行Token量*/
        isstable.modify( issiter, 0, [&]( auto& exa ) 
        {
            exa.iss_count += 1;
            exa.max_quantity += quantity;          
        });
        print("issiter->iss_count:",issiter->iss_count,"|modify end...","issiter->token_contract:",name{issiter->token_contract});
    }
    
    asset issue_quantity = quantity; 
    if(lock_flag == 1)
    {
        /*需要锁仓*/
        
        asset lock_quantity = quantity;
        
        eosio_assert( lock_rate > 0, "invalid lock_rate; must greater than zero." );
        
        /* 计算锁仓数=总发行量*锁仓比例/100 */
        lock_quantity.amount = quantity.amount * lock_rate /100;   
        
        /* 计算实际发行量=总发行量-锁仓数 */        
        issue_quantity.amount -= lock_quantity.amount;
        
        lock_quantity.print();
        issue_quantity.print();
        
        uint64_t time_now = now();
        
        /*计算解锁时间*/
        //uint64_t releaseTime = time_now + lock_time*24*60*60;
        
        /*tag:测试专用，调整为以分钟为单位*/
        uint64_t releaseTime = time_now + lock_time*60;
        
        print("|time_now:",time_now,"|releaseTime:",releaseTime,"|lock begin...");
        
        /*登记锁仓*/
        _locks.token_lock(issiter->token_contract, issue, lock_quantity, releaseTime);

        /*累计锁仓代币数*/
        isstable.modify( issiter, 0, [&]( auto& exa ) 
        {
            exa.lock_quantity += lock_quantity;
        });
        
        /*锁仓(把锁仓的代币转发给ticdist.token合约)*/
        action(
            /*permission_level的功能是对 调用的合约action赋权限
               所以permission_level的第一个参数的用户需要映射active权限给本合约的eosio.code
            */
            permission_level{_self, N(active)},
            issiter->token_contract, N(issue),
            std::make_tuple(_self, lock_quantity, memo)
        ).send();         
    }
    
    print("|issue_quantity:",issue_quantity.amount, "***");

    /*调用代币合约发行代币给issue(未锁仓部份)*/
    action(
        /*permission_level的功能是对 调用的合约action赋权限
           所以permission_level的第一个参数的用户需要映射active权限给本合约的eosio.code
        */
        permission_level{_self, N(active)},
        issiter->token_contract, N(issue),
        std::make_tuple(issue, issue_quantity, memo)
    ).send();
    
    print("call tic.token->create end...");
}

void TicDistToken::addissue(account_name addissue, asset token, string memo )
{
    require_auth( addissue );
    
    issuernumst isstable(_self,_self);
    
    const auto& issinfo = isstable.get( token.symbol.name(), "need to call setconfig..." );
    
    issinfo.max_quantity.print();
    issinfo.quantity.print();   
    
    time_t t = now();
        
    struct tm *curr_ptr;
    curr_ptr = gmtime(&t);

    print("t:",t,"year:",curr_ptr->tm_year+1900,"|month:",curr_ptr->tm_mon+1, "|day:",curr_ptr->tm_mday, "|hour:", curr_ptr->tm_hour);
    
    /*控制在0点到6点之间调用*/
    /*tag: 测试*/
    //eosio_assert( curr_ptr->tm_hour >= 0 && curr_ptr->tm_hour <= 6, "between 0 and 6 call addissue ..." );
    
    //GMT时间转换为本地时间
    int local_hour = curr_ptr->tm_hour+8;
    eosio_assert( local_hour >= 9 && local_hour <= 18, "between 0 and 6 call addissue ..." ); 
    
    /*控制一天内只允许调用一次*/
    /*tag: 测试*/
    //eosio_assert( t > issinfo.add_time + 6*60*60, "one call per day is allowed..." );  
    eosio_assert( t > issinfo.add_time + 60, "one call per day is allowed..." );   

    asset quantity = issinfo.max_quantity * issinfo.dividend_rate / issinfo.divisor_rate;

    /*累计issue接口调用次数以及初始发行Token量*/
    isstable.modify( issinfo, 0, [&]( auto& exa )
    {
        exa.max_quantity += quantity;
        exa.quantity = quantity;
        exa.add_count += 1;
        exa.add_time = t;
    });
    
    print("issinfo.token_contract:", issinfo.token_contract);
    action(
        permission_level{_self, N(active)},
        issinfo.token_contract, N(issue),
        std::make_tuple(addissue, quantity, memo)
    ).send();    
}

void TicDistToken::unlock(account_name unlocker, string memo)
{
    require_auth( unlocker );

    struct lockinfst lockinf;
    /*检查是否过了锁仓时间*/
    _locks.token_unlock(unlocker, &lockinf);

    issuernumst isstable(_self,_self);
    
    const auto& issinfo = isstable.get( lockinf.quantity.symbol.name(), "need to call setconfig..." );
    
    issinfo.lock_quantity.print();
    lockinf.quantity.print();
    eosio_assert( issinfo.lock_quantity >= lockinf.quantity, "big error..." ); 

    /*累计issue接口调用次数以及初始发行Token量*/
    isstable.modify( issinfo, 0, [&]( auto& exa )
    {
        exa.lock_quantity -= lockinf.quantity;
    });
    
    print("token_contract:",lockinf.token_contract,"|***");    

    /*过了锁仓时间,解锁(ticdist.token合约向unlocker转账锁仓的Token)*/
    action(
        permission_level{_self, N(active)},
        lockinf.token_contract, N(transfer),
        std::make_tuple(_self, unlocker, lockinf.quantity, memo)
    ).send();

    print("call unlock end...");
}

void TicDistToken::setconfig(account_name token_contract, asset token, uint64_t issuernum, uint64_t dividend_rate, uint64_t divisor_rate)
{
    require_auth( _self );
    
    /*通胀率的除数必须是100的倍数*/
    eosio_assert( divisor_rate%100 == 0, "divisor_rate must be a multiple of hundred..." );
    
    /*通胀率不能大于1*/
    eosio_assert( dividend_rate <= divisor_rate, "divisor_rate must be greater than dividend_rate..." );
    
    issuernumst isstable(_self,_self);
    auto issiter = isstable.find( token.symbol.name());
    if( issiter == isstable.end() ) 
    {
        /*首次调用，配置issnum.max_num*/
        isstable.emplace(_self, [&]( auto& exa )
        {
            asset zero = token;
            zero.amount = 0;
            
            exa.max_quantity = zero;            
            exa.iss_count = 0;
            exa.max_num = issuernum;

            exa.dividend_rate = dividend_rate;
            exa.divisor_rate = divisor_rate;
            exa.quantity = zero;
            exa.add_count = 0;
            exa.add_time = 0;
            exa.destroy_quantity = zero;
            exa.lock_quantity = zero;
            
            exa.token_contract = token_contract;    
        });
        print("|emplace end...***");
    }
    else 
    {
        eosio_assert( false, "setconfig had been call.." );
    }
    
    print("call setconfig end...");
}

void TicDistToken::destroy( asset quantity, string memo  )
{
    require_auth( _self );
    
    eosio_assert( quantity.amount > 0, "quantity must greater than 0..." );
    issuernumst isstable(_self,_self);
    
    const auto& issinfo = isstable.get( quantity.symbol.name(), "need to call setconfig..." );
    
    issinfo.destroy_quantity.print();
    
    /*查询_self合约的余额*/
    asset fromBalance = eosio::token(issinfo.token_contract).get_balance(_self, quantity.symbol.name());
        
    fromBalance.print();
    /*校验_self合约的代币数-锁仓代币数 > 销毁代币数*/
    eosio_assert( fromBalance-issinfo.lock_quantity>=quantity, "overdrawn balance" );
    
    /*累计issue接口调用次数以及初始发行Token量*/
    isstable.modify( issinfo, 0, [&]( auto& exa )
    {
        exa.destroy_quantity += quantity;
        exa.max_quantity -= quantity;
    });
    
    /*销毁代币(把代币transfer到代币合约)*/
    action(
        permission_level{_self, N(active)},
        issinfo.token_contract, N(transfer),
        std::make_tuple(_self, issinfo.token_contract, quantity, memo)
    ).send();
}

EOSIO_ABI( TicDistToken, (issue)(unlock)(addissue)(setconfig)(destroy) )
