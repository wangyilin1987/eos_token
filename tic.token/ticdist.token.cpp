/**
 *  @file ���ҿ��ƺ�Լ
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

    /*У�����ֲ���*/    
    eosio_assert( lock_flag == 0 || lock_flag == 1, "invalid lock_flag" );
    eosio_assert( lock_rate <= 100, "invalid lock_rate" );
    
    /*�������ֻ������10��*/
    eosio_assert( lock_time <= 3659, "invalid lock_time" );

    /*У��issue���ô����Ƿ񳬹�����(issuernum)*/
    issuernumst isstable(_self,_self);
    auto issiter = isstable.find( quantity.symbol.name());
    if( issiter == isstable.end() ) 
    {
        eosio_assert( false, "need to call setconfig..." );
    }
    else 
    {        
        print("issiter->iss_count:",issiter->iss_count,"|issiter->max_num:",issiter->max_num);
        
        /*У��issue�ӿڵ��ô����Ƿ񳬹�����(issnum.max_num)*/
        eosio_assert( (issiter->iss_count+1) <= issiter->max_num, "call issue time over max" );

        /*�ۼ�issue�ӿڵ��ô����Լ���ʼ����Token��*/
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
        /*��Ҫ����*/
        
        asset lock_quantity = quantity;
        
        eosio_assert( lock_rate > 0, "invalid lock_rate; must greater than zero." );
        
        /* ����������=�ܷ�����*���ֱ���/100 */
        lock_quantity.amount = quantity.amount * lock_rate /100;   
        
        /* ����ʵ�ʷ�����=�ܷ�����-������ */        
        issue_quantity.amount -= lock_quantity.amount;
        
        lock_quantity.print();
        issue_quantity.print();
        
        uint64_t time_now = now();
        
        /*�������ʱ��*/
        //uint64_t releaseTime = time_now + lock_time*24*60*60;
        
        /*tag:����ר�ã�����Ϊ�Է���Ϊ��λ*/
        uint64_t releaseTime = time_now + lock_time*60;
        
        print("|time_now:",time_now,"|releaseTime:",releaseTime,"|lock begin...");
        
        /*�Ǽ�����*/
        _locks.token_lock(issiter->token_contract, issue, lock_quantity, releaseTime);

        /*�ۼ����ִ�����*/
        isstable.modify( issiter, 0, [&]( auto& exa ) 
        {
            exa.lock_quantity += lock_quantity;
        });
        
        /*����(�����ֵĴ���ת����ticdist.token��Լ)*/
        action(
            /*permission_level�Ĺ����Ƕ� ���õĺ�Լaction��Ȩ��
               ����permission_level�ĵ�һ���������û���Ҫӳ��activeȨ�޸�����Լ��eosio.code
            */
            permission_level{_self, N(active)},
            issiter->token_contract, N(issue),
            std::make_tuple(_self, lock_quantity, memo)
        ).send();         
    }
    
    print("|issue_quantity:",issue_quantity.amount, "***");

    /*���ô��Һ�Լ���д��Ҹ�issue(δ���ֲ���)*/
    action(
        /*permission_level�Ĺ����Ƕ� ���õĺ�Լaction��Ȩ��
           ����permission_level�ĵ�һ���������û���Ҫӳ��activeȨ�޸�����Լ��eosio.code
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
    
    /*������0�㵽6��֮�����*/
    /*tag: ����*/
    //eosio_assert( curr_ptr->tm_hour >= 0 && curr_ptr->tm_hour <= 6, "between 0 and 6 call addissue ..." );
    
    //GMTʱ��ת��Ϊ����ʱ��
    int local_hour = curr_ptr->tm_hour+8;
    eosio_assert( local_hour >= 9 && local_hour <= 18, "between 0 and 6 call addissue ..." ); 
    
    /*����һ����ֻ�������һ��*/
    /*tag: ����*/
    //eosio_assert( t > issinfo.add_time + 6*60*60, "one call per day is allowed..." );  
    eosio_assert( t > issinfo.add_time + 60, "one call per day is allowed..." );   

    asset quantity = issinfo.max_quantity * issinfo.dividend_rate / issinfo.divisor_rate;

    /*�ۼ�issue�ӿڵ��ô����Լ���ʼ����Token��*/
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
    /*����Ƿ��������ʱ��*/
    _locks.token_unlock(unlocker, &lockinf);

    issuernumst isstable(_self,_self);
    
    const auto& issinfo = isstable.get( lockinf.quantity.symbol.name(), "need to call setconfig..." );
    
    issinfo.lock_quantity.print();
    lockinf.quantity.print();
    eosio_assert( issinfo.lock_quantity >= lockinf.quantity, "big error..." ); 

    /*�ۼ�issue�ӿڵ��ô����Լ���ʼ����Token��*/
    isstable.modify( issinfo, 0, [&]( auto& exa )
    {
        exa.lock_quantity -= lockinf.quantity;
    });
    
    print("token_contract:",lockinf.token_contract,"|***");    

    /*��������ʱ��,����(ticdist.token��Լ��unlockerת�����ֵ�Token)*/
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
    
    /*ͨ���ʵĳ���������100�ı���*/
    eosio_assert( divisor_rate%100 == 0, "divisor_rate must be a multiple of hundred..." );
    
    /*ͨ���ʲ��ܴ���1*/
    eosio_assert( dividend_rate <= divisor_rate, "divisor_rate must be greater than dividend_rate..." );
    
    issuernumst isstable(_self,_self);
    auto issiter = isstable.find( token.symbol.name());
    if( issiter == isstable.end() ) 
    {
        /*�״ε��ã�����issnum.max_num*/
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
    
    /*��ѯ_self��Լ�����*/
    asset fromBalance = eosio::token(issinfo.token_contract).get_balance(_self, quantity.symbol.name());
        
    fromBalance.print();
    /*У��_self��Լ�Ĵ�����-���ִ����� > ���ٴ�����*/
    eosio_assert( fromBalance-issinfo.lock_quantity>=quantity, "overdrawn balance" );
    
    /*�ۼ�issue�ӿڵ��ô����Լ���ʼ����Token��*/
    isstable.modify( issinfo, 0, [&]( auto& exa )
    {
        exa.destroy_quantity += quantity;
        exa.max_quantity -= quantity;
    });
    
    /*���ٴ���(�Ѵ���transfer�����Һ�Լ)*/
    action(
        permission_level{_self, N(active)},
        issinfo.token_contract, N(transfer),
        std::make_tuple(_self, issinfo.token_contract, quantity, memo)
    ).send();
}

EOSIO_ABI( TicDistToken, (issue)(unlock)(addissue)(setconfig)(destroy) )
