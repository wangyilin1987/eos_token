/**
 *  @file ����
 *  @copyright wyl
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

#include <string>
using namespace eosio;
using std::string;

/*��Ϊ�ӿڷ���ֵ*/ 
struct lockinfst
{
    account_name      token_contract;
    asset             quantity;
    
    EOSLIB_SERIALIZE( lockinfst, (token_contract)(quantity) )
};

class TokenTimeLock : public contract {
  public:
    /**
      * FUNC: ���캯��
      * INPUT:                             
      *      self: ��Լӵ�����˻�
      * OUTPUT:                        
      * RETURN: 
      * DESC:                            
    */
    TokenTimeLock( account_name self ):contract(self){}

    /**
      * FUNC: �Ǽ�����
      * INPUT:
      *      token_contract:Token��Լ
      *      locker: �����û�
      *      lock_quantity: ����Token��
      *      releaseTime������ʱ��(1970�굽����ʱ��ʱ������)
      * OUTPUT:                        
      * RETURN: 
      * DESC:
    */
    void token_lock(account_name token_contract, account_name locker, asset lock_quantity, uint64_t release_time);

    /**
      * FUNC: ����
      * INPUT:
            locker: �����û�
      * OUTPUT:
      *     ret: ������Ϣ:���ֵ�Token��Լ�����ֵ�Token��
      * RETURN: 
      * DESC:
    */
    void token_unlock(account_name unlocker, struct lockinfst *ret);

  private:

    /*���ֱ�*/
    ///@abi table locktb i64
    struct lockst 
    {
        account_name      locker;           /*�����û�*/
        account_name      token_contract;   /*���Һ�Լ*/
        asset             quantity;         /*����Token����*/
        uint64_t          release_time;     /*����ʱ��*/

        uint64_t primary_key()const { return locker; }
    };

    typedef eosio::multi_index<N(locktb), lockst> locks;
};


;

