/**
 *  @file ���ҿ��ƺ�Լ
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
      * FUNC: ���캯��
      * INPUT:
      *      self: ��Լӵ�����˻�
      * OUTPUT:                        
      * RETURN: 
      * DESC:                            
    */
    TicDistToken( account_name self ):contract(self),_locks(self){}


    /**
      * FUNC: ��ʼ���д��Һ���
      * INPUT:                           
      *      issue: ���з�A
      *      quantity����ʼ���и����з�A��Token��
      *      lock_flag: ���ֱ�־(0:�����֣�1:����)
      *      lock_rate�����ֱ���(eg:50��ʾ����50%)
      *      lock_time: ����ʱ�䣨��λ:�죩
      *      memo: ��ע
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
      * FUNC: ��������
      * INPUT:
      *      addissue: �������ҽ��շ�
      *      token: ������Token
      *      memo: ��ע
      * OUTPUT:
      * RETURN:  
      * DESC:
    */
    void addissue( account_name addissue, asset token, string memo );
    
    /**
      * FUNC: ���ֽ���
      * INPUT:
      *      locker: �����˻�
      *      memo: ��ע
      * OUTPUT:
      * RETURN:
      * DESC:
    */
    void unlock(account_name unlocker, string memo);
     
    /**
      * FUNC: ���в�������
      * INPUT:
      *      token_contract:���Һ�Լ  
      *      token:���е�Token
      *      issuernum: ��ʼ���з�����
      *      dividend_rate: ͨ����(��)�ı�����
      *      divisor_rate: ͨ����(��)�ĳ���
      * OUTPUT:
      * RETURN:  
      * DESC:
    */
    void setconfig(account_name token_contract, asset token, uint64_t issuernum, uint64_t dividend_rate, uint64_t divisor_rate);

    /**
      * FUNC: ��������
      * INPUT:
      *      quantity:���ٵĴ�������
      *      memo: ��ע
      * OUTPUT:
      * RETURN:  
      * DESC:�����ٵĴ���transfer�����Һ�Լ
    */
    void destroy( asset quantity, string memo  );
     
    private:

    TokenTimeLock _locks;

    /*�������Ʊ�*/
    ///@abi table issuernumtb i64
    struct issnum {
        account_name      token_contract;/*���Һ�Լ*/
        asset             max_quantity;  /*�ۼƷ��е�Token��*/
        uint64_t          iss_count;     /*���ù�issue�ӿڵĴ���*/
        uint64_t          max_num;       /*�������issue�ӿڵ�������*/

        uint64_t          dividend_rate; /*ͨ����(��)�ı�����*/
        uint64_t          divisor_rate;  /*ͨ����(��)�ĳ���*/
        asset             quantity;      /*���һ��������Token��*/
        uint64_t          add_count;     /*add_issue�ӿڵ��ô���*/
        time_t            add_time;      /*���һ�ε���ʱ��*/
        
        asset             destroy_quantity; /*���ٵ�Token��*/
        asset             lock_quantity;    /*���ֵ�Token��*/

        uint64_t primary_key()const { return quantity.symbol.name(); }
    };
    
    typedef eosio::multi_index<N(issuernumtb), issnum> issuernumst;
};

