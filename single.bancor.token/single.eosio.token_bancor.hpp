/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/currency.hpp>
#include <eosiolib/transaction.hpp>

#include <fc/string.hpp>
#include <vector>
#include <string>
#include <cmath>

namespace eosiosystem {
   class system_contract;
   
   using namespace eosio;
   using eosio::asset;
   using eosio::symbol_type;

   typedef double real_type;

   /**
    *  Uses Bancor math to create a 50/50 relay between two asset types. The state of the
    *  bancor exchange is entirely contained within this struct. There are no external
    *  side effects associated with using this API.
    */
   ///@abi table banmarket i64
   struct exchange_state {
      uint64_t key;
      asset    supply;
      asset    virtual_quote;           /*�����Ѻ�Ĵ�����*/
      asset    pre_quote;               /*�ڳ�ʵ�ʵ�Ѻ�Ĵ�����*/           
      asset    pre_base;                /*�ڳ﷢�е�Token*/
      asset    pre_base_bal;            /*���ڵ����ڳ��sellʱ����۸�ķ�����*/
      account_name    limit_admin;      /*ӵ��һ������Ȩ�޵Ĺ���Ա*/

      struct connector {
         account_name  contract;
         asset balance;
         double weight = .5;

         EOSLIB_SERIALIZE( connector, (contract)(balance)(weight) )
      };

      connector base;
      connector quote;

      uint64_t primary_key()const { return key;}

      asset convert_to_exchange( asset in ); 
      asset convert_from_exchange( asset in );
      asset convert( asset from, symbol_type to );

      EOSLIB_SERIALIZE( exchange_state, (key)(supply)(virtual_quote)(pre_quote)(pre_base)(pre_base_bal)(limit_admin)(base)(quote) )
   };

   typedef eosio::multi_index<N(banmarket), exchange_state> markets;

} /// namespace eosiosystem


namespace eosio {

   using std::string;
   using eosiosystem::markets;   

   class token : public contract {      
      public:
         token( account_name self ):contract(self),whitelists_table(_self,_self){}

        /**
          * FUNC:create
          * INPUT:
          *      maximum_supply: ����token�����ͨ��
          *      quantity: Ԥ�����е���Լ�Ĵ���,Ŀǰ��0
          *      cw���㶨������;ȡֵ��Χ(0,1000]
          *      quote_contract:��������Һ�Լ
          *      quote_supply:�ܵ�Ѻ�Ĵ�����(���ڶ���)
          *      pre_quote:ʵ�ʵ�Ѻ�Ĵ�����
          *      pre_releasea:Ԥ������Ŀ�����е�����Token��
          *      pre_releaseb:Ԥ�����ڳ������Token��
          *      receiver:Ԥ���е�����Token�����˺�(ͬʱ������Ϊ����Ա)
          * OUTPUT:
          * RETURN:
          * DESC:��������token
        */
        // @abi action
         void create( asset        maximum_supply,
                      asset        quantity,
                      uint64_t     cw,
                      account_name quote_contract,
                      asset        quote_supply,
                      asset        pre_quote,
                      asset        pre_releasea,
                      asset        pre_releaseb,
                      account_name receiver
                      );

        /**
          * FUNC:setconfig
          * INPUT:
          *      admin: create�ӿ�ָ���Ĺ���Ա
          *      quote_quantity: ���ӵĴ�����
          *      cw���㶨������;ȡֵ��Χ(0,1000)
          *      symbol:��������(eg:EOS)
          * OUTPUT:
          * RETURN:
          * DESC:���ò���(Ŀǰֻ֧���޸ĺ㶨������,���Ӵ������,�������ù���Ա)
        */
        // @abi action
         void setconfig(account_name admin, asset quote_quantity, uint64_t cw, string symbol);

        /**
          * FUNC:issue
          * INPUT:
          *      to:       �����������ܴ��ҵ��˺�
          *      quantity�����������ܴ������� 
          *      memo:��ע
          * OUTPUT:
          * RETURN:
          * DESC:�������ܴ���
        */
        // @abi action
         void issue( account_name to, asset quantity, string memo );
         
         /*�ڲ����õ��������ܴ��ҽӿ�*/
         void in_issue( asset quantity, string memo );
         
        /**
          * FUNC:retire
          * INPUT:
          *      quantity��ȼ�յ����ܴ������� 
          *      memo:��ע
          * OUTPUT:
          * RETURN:
          * DESC:ȼ�����ܴ���
        */
        // @abi action
         void retire( asset quantity, string memo );

         /*�ڲ����õ�ȼ�����ܴ��ҽӿ�*/
         void in_retire( asset quantity, string memo );
         
        /**
          * FUNC:destroy
          * INPUT:
          *      receiver: ����������˺�
          *      symbol�����ܴ�������
          *      memo:��ע
          * OUTPUT:
          * RETURN:
          * DESC:�������ܴ���
        */
        // @abi action
         void destroy ( account_name receiver, string symbol, string memo );
        
        // @abi action
         void transfer( account_name from,
                        account_name to,
                        asset        quantity,
                        string       memo );
                        
         /*�ڲ����õ�transfer�ӿ�(����from����Ȩ��֪ͨ)*/               
         void in_transfer( account_name from,
                        account_name to,
                        asset        quantity,
                        string       memo );
            
        // @abi action
         void close( account_name owner, string symbol );
         
        /**
          * FUNC:regadminwhite
          * INPUT:
          *      admin������������Ա
          *      newadmin���������¹���Ա
          * OUTPUT:
          * RETURN:
          * DESC:���/�ƽ�����������Ա
        */
        // @abi action
         void regadmwhite( account_name admin, account_name newadmin );
                  
        /**
          * FUNC:regwhitelist
          * INPUT:
          *      admin��   ����������Ա
          *      contract����ӽ��������ĺ�Լ�˻�
          *      enabled:  true:��Ч;false:ʧЧ
          * OUTPUT:
          * RETURN:
          * DESC:���/ɾ���������Ұ�����
        */
        // @abi action
         void regwhitelist( account_name admin, account_name contract, bool enabled );
         
        // @abi action
         void removewhite( account_name contract );

        void apply( account_name contract, account_name act );
        
        /**
          * FUNC:tic_symbol_to_string
          * INPUT:
          *      symbol��symbol_type���͵Ĵ�������
          * OUTPUT:
          *      token:string���͵Ĵ�������
          * RETURN:
          * DESC:������������ת��(symbol_type->string)
        */
         void tic_symbol_to_string(symbol_type symbol, string &token);
    
         inline asset get_supply( symbol_name sym )const;

         inline asset get_balance( account_name owner, symbol_name sym )const;
         inline asset get_balance( account_name contract, account_name owner, symbol_name sym );
         
      private:
         ///@abi table accounts i64
         struct account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.name(); }
         };

         ///@abi table stat i64
         struct cstats {
            asset          supply;
            asset          max_supply;
            account_name   issuer;

            uint64_t primary_key()const { return supply.symbol.name(); }
         };
         
         // @abi table whitelist
         struct whitelist
         {
             account_name contract;
             bool         created;
             bool         enabled;
             bool         isadmin;
             uint64_t primary_key() const { return contract; }
             EOSLIB_SERIALIZE( whitelist, (contract)(created)(enabled)(isadmin) )
         };
         
         typedef eosio::multi_index<N(accounts), account> accounts;
         typedef eosio::multi_index<N(stat), cstats> stats;
         eosio::multi_index<N(whitelist), whitelist> whitelists_table;

         void sub_balance( account_name owner, asset value );
         void add_balance( account_name owner, asset value, account_name ram_payer );
         void on( const currency::transfer& t, account_name code );

        typedef struct memoStructure 
        {
            string   trade_flag;
            string   target_symbol;
        }stmemoStructure;

         vector<string> split(const string& str, const string& delim);
         stmemoStructure parseMemo(string memo);

      public:
         struct transfer_args {
            account_name  from;
            account_name  to;
            asset         quantity;
            string        memo;
         };
   };

   asset token::get_supply( symbol_name sym )const
   {
      stats statstable( _self, sym );
      const auto& st = statstable.get( sym );
      return st.supply;
   }

   asset token::get_balance( account_name owner, symbol_name sym )const
   {
      accounts accountstable( _self, owner );
      const auto& ac = accountstable.get( sym );
      return ac.balance;
   }
   
   asset token::get_balance( account_name contract, account_name owner, symbol_name sym )
   {
     accounts accountstable( contract, owner );
     const auto& ac = accountstable.get( sym );
     return ac.balance;
   }

} /// namespace eosio
