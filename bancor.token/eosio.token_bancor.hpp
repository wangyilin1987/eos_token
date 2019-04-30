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
      asset    virtual_quote;           /*虚拟抵押的储备金*/
      asset    pre_quote;               /*众筹实际抵押的储备金*/           
      asset    pre_base;                /*众筹发行的Token*/
      asset    pre_base_bal;            /*用于低于众筹价sell时计算价格的发行量*/
      account_name    limit_admin;      /*拥有一定操作权限的管理员*/
      uint64_t create_time;             /*代币创建时间*/
      asset    supply_base_at_crete;       /*智能代币初次发行时的发行量*/

      struct connector {
         account_name  contract;
         asset balance;
         double weight = .5;

         EOSLIB_SERIALIZE( connector, (contract)(balance)(weight) )
      };

      connector base;
      connector quote;

      uint64_t primary_key()const { return key;}

      asset convert_to_exchange( connector& c, asset in ); 
      asset convert_from_exchange( connector& c, asset in );
      asset convert( asset from, symbol_type to );

      EOSLIB_SERIALIZE( exchange_state, (key)(supply)(virtual_quote)(pre_quote)(pre_base)(pre_base_bal)(limit_admin)(create_time)(supply_base_at_crete)(base)(quote) )
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
          *      maximum_supply: 智能token最大流通量
          *      quantity: 初始发行到合约的智能token数量
          *      cw：恒定储备率;取值范围(0,1000]
          *      quote_contract:储备金代币合约
          *      quote_supply:总抵押的储备金(用于定价)
          *      pre_quote:实际抵押的储备金
          *      pre_releasea:预发行项目方持有的智能Token数
          *      pre_releaseb:预发行众筹的智能Token数
          *      receiver:预发行的智能Token接收账号(同时被设置为管理员)
          * OUTPUT:
          * RETURN:
          * DESC:发行智能token
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
          *      admin: create接口指定的管理员
          *      quote_quantity: 增加的储备金
          *      cw：恒定储备率;取值范围(0,1000)
          *      symbol:代币名称(eg:EOS)
          * OUTPUT:
          * RETURN:
          * DESC:设置参数(目前只支持修改恒定储备率,增加储备金池,重新设置管理员)
        */
        // @abi action
         void setconfig(account_name admin, asset quote_quantity, uint64_t cw, string symbol);

        /**
          * FUNC:issue
          * INPUT:
          *      quantity：增发的智能代币数量 
          *      memo:备注
          * OUTPUT:
          * RETURN:
          * DESC:增发智能代币
        */
        // @abi action
         void issue( asset quantity, string memo );
         
         /*内部调用的增发智能代币接口*/
         void in_issue( asset quantity, string memo );
         
        /**
          * FUNC:retire
          * INPUT:
          *      quantity：燃烧的智能代币数量 
          *      memo:备注
          * OUTPUT:
          * RETURN:
          * DESC:燃烧智能代币
        */
        // @abi action
         void retire( asset quantity, string memo );

         /*内部调用的燃烧智能代币接口*/
         void in_retire( asset quantity, string memo );
         
        /**
          * FUNC:destroy
          * INPUT:
          *      receiver: 储备金接收账号
          *      symbol：智能代币名称
          *      memo:备注
          * OUTPUT:
          * RETURN:
          * DESC:销毁智能代币
        */
        // @abi action
         void destroy ( account_name receiver, string symbol, string memo );
        
        // @abi action
         void transfer( account_name from,
                        account_name to,
                        asset        quantity,
                        string       memo );
                        
         /*内部调用的transfer接口(屏蔽from的授权和通知)*/               
         void in_transfer( account_name from,
                        account_name to,
                        asset        quantity,
                        string       memo );
            
        // @abi action
         void close( account_name owner, string symbol );
         
        /**
          * FUNC:regadminwhite
          * INPUT:
          *      admin：白名单管理员
          *      newadmin：白名单新管理员
          * OUTPUT:
          * RETURN:
          * DESC:添加/移交白名单管理员
        */
        // @abi action
         void regadmwhite( account_name admin, account_name newadmin );
                  
        /**
          * FUNC:regwhitelist
          * INPUT:
          *      admin：   白名单管理员
          *      contract：添加进白名单的合约账户
          *      enabled:  true:生效;false:失效
          * OUTPUT:
          * RETURN:
          * DESC:添加/删除创建代币白名单
        */
        // @abi action
         void regwhitelist( account_name admin, account_name contract, bool enabled );
         
         void removewhite( account_name contract );

         void apply( account_name contract, account_name act );

        /**
          * FUNC:precalculate
          * INPUT:
          *      quantity：   源Token数量
          *      dest_symbol：目标token代币
          *      isbuy:  true:购买智能Token;false:出售智能Token
          * OUTPUT:
          * RETURN:
          * DESC:预先计算兑换的Token数量
        */
        // @abi action
         void precalculate( asset        quantity,
                            string       dest_symbol,
                            bool isbuy);
         
        /**
          * FUNC:tic_symbol_to_string
          * INPUT:
          *      symbol：symbol_type类型的代币名称
          * OUTPUT:
          *      token:string类型的代币名称
          * RETURN:
          * DESC:代币名称类型转换(symbol_type->string)
        */
         void tic_symbol_to_string(symbol_type symbol, string &token);
         
         void if_error_return(bool is_true, uint16_t code, const std::string msg) 
         {
            // 什么都不用做
            if (is_true) 
            {
                return;
            }
            // 有错误，需要中断返回
            std::string result = "{\"success\":false, \"code\":";
            result += std::to_string(code);
            result += ", \"message\":\"";
            result += msg;
            result += "\"}";
            eosio_assert(false, result.c_str());
         }
    
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
