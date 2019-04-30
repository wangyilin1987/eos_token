/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include "eosio.token_bancor.hpp"

namespace eosiosystem {
   asset exchange_state::convert_to_exchange( connector& c, asset in ) {

      real_type R(supply.amount);
      real_type C(c.balance.amount+in.amount);
      real_type F(c.weight/1000.0);
      real_type T(in.amount);
      real_type ONE(1.0);

      real_type E = -R * (ONE - std::pow( ONE + T / C, F) );
      //print( "E: ", E, "\n");
      int64_t issued = int64_t(E);

      supply.amount += issued;
      c.balance.amount += in.amount;

      return asset( issued, supply.symbol );
   }

   asset exchange_state::convert_from_exchange( connector& c, asset in ) {
      eosio_assert( in.symbol== supply.symbol, "unexpected asset symbol input" );

      real_type R(supply.amount - in.amount);
      real_type C(c.balance.amount);
      real_type F(1000.0/c.weight);
      real_type E(in.amount);
      real_type ONE(1.0);


     // potentially more accurate: 
     // The functions std::expm1 and std::log1p are useful for financial calculations, for example, 
     // when calculating small daily interest rates: (1+x)n
     // -1 can be expressed as std::expm1(n * std::log1p(x)). 
     // real_type T = C * std::expm1( F * std::log1p(E/R) );
      
      real_type T = C * (std::pow( ONE + E/R, F) - ONE);
      //print( "T: ", T, "\n");
      int64_t out = int64_t(T);

      supply.amount -= in.amount;
      c.balance.amount -= out;

      return asset( out, c.balance.symbol );
   }

   asset exchange_state::convert( asset from, symbol_type to ) {
      auto sell_symbol  = from.symbol;
      auto ex_symbol    = supply.symbol;
      auto base_symbol  = base.balance.symbol;
      auto quote_symbol = quote.balance.symbol;

      //print( "From: ", from, " TO ", asset( 0,to), "\n" );
      //print( "base: ", base_symbol, "\n" );
      //print( "quote: ", quote_symbol, "\n" );
      //print( "sell_symbol: ", sell_symbol, "\n" );
      //print( "ex: ", supply.symbol, "\n" );

      if( sell_symbol != ex_symbol ) {
         if( sell_symbol == base_symbol ) {
            from = convert_to_exchange( base, from );
         } else if( sell_symbol == quote_symbol ) {
            from = convert_to_exchange( quote, from );
         } else { 
            eosio_assert( false, "invalid sell" );
         }
      } else {
         if( to == base_symbol ) {
            from = convert_from_exchange( base, from ); 
         } else if( to == quote_symbol ) {
            from = convert_from_exchange( quote, from ); 
         } else {
            eosio_assert( false, "invalid conversion" );
         }
      }

      if( to != from.symbol )
         return convert( from, to );

      return from;
   }
} /// namespace eosiosystem

namespace eosio {
void token::tic_symbol_to_string(symbol_type symbol, string &token)
{
     auto sym = symbol.value;
     sym >>= 8;
     for( int i = 0; i < 7; ++i ) {
        char c = (char)(sym & 0xff);
        if( !c ) return;
        token.push_back(c);
        sym >>= 8;
     }
}
/*
vector<string> split(const string& str, const string& delim)
{
    vector<string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token = str.substr(prev, pos-prev);
        tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

eosio::token::stmemoStructure token::parseMemo(string memo)
{
    eosio::token::stmemoStructure res;
    auto parts = split(memo, ",");
    res.trade_flag = parts[0];
    res.target_symbol = parts[1];

    return res;
}
*/

void token::create( asset        maximum_supply,
                    asset        quantity,
                    uint64_t     cw,
                    account_name quote_contract,
                    asset        quote_supply,
                    asset        pre_quote,
                    asset        pre_releasea,
                    asset        pre_releaseb,
                    account_name receiver
                  )
{
    require_auth( receiver );

    eosio_assert( receiver != _self, "receiver must not be self");

    /*参数校验*/
    auto sym = maximum_supply.symbol;
    eosio_assert( sym.is_valid(), "maximum_supply invalid symbol name" );
    eosio_assert( maximum_supply.is_valid(), "invalid supply");
    eosio_assert( maximum_supply.amount > 0, "max-supply must be positive");

    auto symq = quantity.symbol;
    eosio_assert( symq.is_valid(), "quantity invalid symbol name" );    
    eosio_assert( quantity.is_valid(), "invalid quantity" );    
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );
    eosio_assert( quantity.symbol == maximum_supply.symbol, "symbol mismatch" );
    eosio_assert( quantity.amount+pre_releasea.amount+pre_releaseb.amount <= maximum_supply.amount, "quantity exceeds available supply");

    auto symqu = quote_supply.symbol;
    eosio_assert( symqu.is_valid(), "quantity invalid symbol name" );    
    eosio_assert( quote_supply.is_valid(), "invalid quantity" );  
    eosio_assert( quote_supply.amount > 0, "must purchase a positive amount" );

    auto sympr = pre_quote.symbol;
    eosio_assert( sympr.is_valid(), "quantity invalid symbol name" );    
    eosio_assert( pre_quote.is_valid(), "invalid quantity" );  
    eosio_assert( pre_quote.symbol == quote_supply.symbol, "pre_quote symbol mismatch" );

    auto symprrea = pre_releasea.symbol;
    eosio_assert( symprrea.is_valid(), "quantity invalid symbol name" );    
    eosio_assert( pre_releasea.is_valid(), "invalid quantity" );  
    eosio_assert( pre_releasea.symbol == quantity.symbol, "pre_releasea symbol mismatch" );

    auto symprreb = pre_releaseb.symbol;
    eosio_assert( symprreb.is_valid(), "quantity invalid symbol name" );    
    eosio_assert( pre_releaseb.is_valid(), "invalid quantity" );  
    eosio_assert( pre_releaseb.symbol == quantity.symbol, "pre_releaseb symbol mismatch" );

    eosio_assert( cw > 0 && cw <= 1000, "cw must in (0,1000]");

    if (cw == 1000)
    {
        /*1:1兑换需要保证储备金余额和智能代币发行量相等*/
        eosio_assert( quantity.amount == quote_supply.amount, "quantity and quote_supply must be equa" );
        eosio_assert( pre_quote.amount == 0 && pre_releasea.amount == 0 && pre_releaseb.amount == 0, "pre_quote && pre_release must be zero" );
    }

    /*校验创建代币账号是否在白名单内*/
    /*  暂时放开白名单控制
    auto itr_white = whitelists_table.find(receiver);
    eosio_assert( itr_white != whitelists_table.end() && itr_white->enabled == true, "receiver not on the whitelist" );

    whitelists_table.modify(itr_white,_self, [&]( auto& s ) {
        s.created = true;
        s.enabled = false;
    });
    */

    stats statstable( _self, sym.name() );
    auto existing = statstable.find( sym.name() );
    eosio_assert( existing == statstable.end(), "token with symbol already exists" );
    
    statstable.emplace( _self, [&]( auto& s ) {
       /*发行到合约的Token+项目方持有的预发行Token*/
       s.supply = quantity + pre_releasea + pre_releaseb;
       s.max_supply    = maximum_supply;
       s.issuer        = _self;
    });
    
    markets _market(_self,_self);
    uint64_t key = maximum_supply.symbol.value;
    
    auto itr = _market.find(key);

    eosio_assert(itr == _market.end(), "token already created");

    if(pre_quote.amount > 0)
    {
        /*填充预发行储备金池*/
        action(
                permission_level{ receiver, N(active) },
                quote_contract, N(transfer),
                std::make_tuple(receiver, _self, pre_quote, std::string("initial.create"))
        ).send();
    }

    /*发行到合约的智能token*/
    add_balance( _self, quantity, _self );
    
    if(pre_releasea.amount+pre_releaseb.amount > 0)
    {
        /*发行到项目方的智能token*/
        add_balance( receiver, pre_releasea+pre_releaseb, receiver );
    }
    
    _market.emplace( _self, [&]( auto& m ) {
        m.key = key;

        /*组装supply的token symbol*/
        uint64_t symboluint;
        string strtoken;
        tic_symbol_to_string(maximum_supply.symbol, strtoken);
        strtoken = strtoken.substr(0,6);
        strtoken.append("C");

        symboluint = ::eosio::string_to_symbol(4,strtoken.c_str());
            
        m.supply.amount = 100000000000000ll;
        m.supply.symbol = symboluint;
        eosio_assert( m.supply.symbol.is_valid(), "m.supply.symbol invalid symbol name" );
        
        m.base.contract = _self;
        m.base.balance = quantity;
        m.base.weight = cw;
        m.quote.contract = quote_contract;
        m.quote.balance = quote_supply;
        
        /*保存定价储备金量*/
        m.virtual_quote = quote_supply - pre_quote;
        
        m.create_time = now();
        m.supply_base_at_crete = quantity + pre_releasea + pre_releaseb;
        
        m.pre_quote = pre_quote;
        m.pre_base = pre_releaseb;
        m.pre_base_bal = pre_releaseb;
        m.limit_admin = receiver;
        m.quote.weight = cw;
    });
}

void token::issue( asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );
    
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    require_auth( st.issuer );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch." );
    eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    uint64_t key = quantity.symbol.value;
    
    markets _market(_self,_self);

    auto itr = _market.find(key);

    eosio_assert(itr != _market.end(), "token does not exist........");
    
    eosio_assert(itr->base.weight != 1000, "weight is 1000, not allow issue");

    eosio_assert( quantity.symbol == itr->base.balance.symbol, "base symbol mismatch" );
    
    /*修改markets表的智能代币发行量*/
    _market.modify( itr, 0, [&]( auto& es ) {
        es.base.balance += quantity;
    });
    
    /*修改发行量*/
    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply += quantity;
    });

    /*增发智能代币*/
    add_balance( st.issuer, quantity, st.issuer );
}

void token::in_issue( asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );
    
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch.." );
    eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    /*修改发行量*/
    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply += quantity;
    });

    /*增发智能代币*/
    add_balance( st.issuer, quantity, st.issuer );
}

void token::retire( asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must retire positive quantity" );
    
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( st.issuer );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch..." );

    /*修改发行量*/
    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply -= quantity;
    });

    uint64_t key = quantity.symbol.value;
    
    markets _market(_self,_self);

    auto itr = _market.find(key);

    eosio_assert(itr != _market.end(), "token does not exist.");
    
    eosio_assert(itr->base.weight != 1000, "weight is 1000, not allow retire");

    eosio_assert( quantity.symbol == itr->base.balance.symbol, "base symbol precision mismatch...." );
    
    /*修改markets表的智能代币发行量*/
    _market.modify( itr, 0, [&]( auto& es ) {
        es.base.balance -= quantity;
    });
    
    /*燃烧智能代币*/
    sub_balance( st.issuer, quantity );
}

void token::in_retire( asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must retire positive quantity" );
    
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch....." );

    /*修改发行量*/
    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply -= quantity;
    });
    
    /*燃烧智能代币*/
    sub_balance( st.issuer, quantity );
}

void token::destroy ( account_name receiver, string symbol, string memo )
{
    uint64_t symboluint;
    /*string类型的代币名称转换为uint64类型*/
    symboluint = ::eosio::string_to_symbol(4,symbol.c_str());
   
    /*通过uint64类型的代币名称初始化symbol_type类型*/
    symbol_type token_symbol(symboluint);
    
    eosio_assert(token_symbol.is_valid(), "invalid token_symbol.....");
 
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );
    
    auto sym_name = token_symbol.name();
    
    accounts accountstable( _self, _self );
    auto itr_account = accountstable.find( sym_name );
    
    eosio_assert( itr_account != accountstable.end(), "accounts with symbol does not exist" );
    
    itr_account->balance.print();

    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist" );
    
    const auto& st = *existing;

    /*校验_self是否拥有发行的所有代币*/
    eosio_assert( itr_account->balance == existing->supply, "_self have not owner all supply balance" );
    
    uint64_t key = token_symbol.value;

    markets _market(_self,_self);

    auto itr = _market.find(key);

    eosio_assert(itr != _market.end(), "token does not exist..");
    require_auth( itr->limit_admin );

    /*eosio_assert( itr_account->balance == itr->base.balance, "_self have not owner all base balance" );*/

    sub_balance( _self, itr_account->balance );
    statstable.erase(existing);    
    _market.erase(itr);

    if(itr->quote.balance.amount-itr->virtual_quote.amount > 0)
    {
        action(
                permission_level{ _self, N(active) },
                itr->quote.contract, N(transfer),
                std::make_tuple(_self, receiver, itr->quote.balance-itr->virtual_quote, std::string("initial.destroy"))
        ).send();
    }
}

void token::transfer( account_name from,
                      account_name to,
                      asset        quantity,
                      string       memo )
{
    eosio_assert( from != to, "cannot transfer to self" );
    require_auth( from );
    eosio_assert( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.name();
    stats statstable( _self, sym );
    const auto& st = statstable.get( sym );

    require_recipient( from );
    require_recipient( to );

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch......" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    sub_balance( from, quantity );
    add_balance( to, quantity, from );
    
    if(to != _self)
    {
        print("normal transfer.\n");
        return;
    }

    if(memo.substr(0,7) == "initial" || memo.empty())
    {
        print("initial.\n");
        return;
    }
    /*--------sell Token 逻辑-----------*/

    markets _market(_self,_self);

    uint128_t key = quantity.symbol.value;

    auto itr = _market.find(key);

    eosio_assert(itr != _market.end(), "token does not exist...");
    

    /*string类型的代币名称转换为symbol_type类型*/
    uint64_t symboluint;
    symboluint = ::eosio::string_to_symbol(4,memo.c_str());
   
    symbol_type token_symbol(symboluint);
       
    print(memo,"\n");    
    token_symbol.print();
    itr->quote.balance.symbol.print();
    
    eosio_assert(token_symbol.is_valid(), "invalid token_symbol.");
    eosio_assert(itr->quote.balance.symbol == token_symbol, "token_symbol incorrect");

    asset token_out(0,itr->quote.balance.symbol);
    
    if(itr->base.weight == 1000)
    {
        /*1:1兑换*/           
        token_out.amount = quantity.amount;
        in_retire(quantity, "retire by _self");
        
        _market.modify( itr, 0, [&]( auto& es ) {
            es.base.balance -= quantity;
            es.quote.balance -= token_out;
        });
    }
    else
    {
        _market.modify( itr, 0, [&]( auto& es ) {
              token_out = es.convert( quantity,  itr->quote.balance.symbol );
        });
        eosio_assert(itr->quote.balance.amount-itr->virtual_quote.amount > 0, "later try again.");
    }
    
    eosio_assert( token_out.amount > 0, "token amount received from selling EOS is too low" );
    
    asset balance = get_balance(itr->quote.contract, itr->base.contract, token_out.symbol.name());
    
    balance.print();
    token_out.print();
    
    eosio_assert( balance >= token_out, "later try again..." );

    /*bancor储备金出账*/
    action(
            permission_level{ itr->base.contract, N(active) },
            itr->quote.contract, N(transfer),
            std::make_tuple(itr->base.contract, from, token_out, memo)
    ).send();
}

void token::in_transfer( account_name from,
                      account_name to,
                      asset        quantity,
                      string       memo )
{
    eosio_assert( from != to, "cannot transfer to self" );
    eosio_assert( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.name();
    stats statstable( _self, sym );
    const auto& st = statstable.get( sym );
    
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch......." );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    sub_balance( from, quantity );
    add_balance( to, quantity, from );
}

void token::sub_balance( account_name owner, asset value ) {
   accounts from_acnts( _self, owner );

   const auto& from = from_acnts.get( value.symbol.name(), "no balance object found" );
   eosio_assert( from.balance.amount >= value.amount, "overdrawn balance" );

  from_acnts.modify( from, owner, [&]( auto& a ) {
      a.balance -= value;
  });
}

void token::add_balance( account_name owner, asset value, account_name ram_payer )
{
   accounts to_acnts( _self, owner );
   auto to = to_acnts.find( value.symbol.name() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, 0, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

void token::precalculate(asset        quantity,
                         string       dest_symbol,
                         bool         isbuy)
{
    eosio_assert(quantity.amount > 0, "must purchase a positive amount" );

    /*string类型的代币名称转换为symbol_type类型*/
    uint64_t symboluint;
    symboluint = ::eosio::string_to_symbol(4,dest_symbol.c_str());
   
    symbol_type token_symbol(symboluint);
    
    eosio_assert(token_symbol.is_valid(), "invalid token_symbol..");
    asset token_out;
    if (isbuy == true)
    {
        uint64_t key = token_symbol.value;
        /*计算购买智能代币数量*/
        markets _market(_self,_self);

        auto itr = _market.find(key);

        eosio_assert(itr != _market.end(), "token does not exist....");
        eosio_assert(itr->quote.balance.symbol.value == quantity.symbol.value, "quote.balance.symbol mismatching....");
  
        if(itr->base.weight == 1000)
        {
            /*1:1兑换*/           
            token_out = quantity;
        }
        else
        {
            _market.modify( itr, 0, [&]( auto& es ) {
                token_out = es.convert( quantity,  token_symbol );
            }); 
        }          
    }
    else
    {
        eosio_assert(quantity.symbol.is_valid(), "invalid quantity.symbol");
        eosio_assert(quantity.amount > 0, "must purchase a positive amount" );

        markets _market(_self,_self);

        uint128_t key = quantity.symbol.value;

        auto itr = _market.find(key);

        eosio_assert(itr != _market.end(), "token does not exist.....");
        eosio_assert(itr->quote.balance.symbol.value == token_symbol.value, "base.balance.symbol mismatching....");

        if(itr->base.weight == 1000)
        {
            /*1:1兑换*/           
            token_out = quantity;
        }
        else
        {
            /*
            _market.modify( itr, 0, [&]( auto& es ) {
                  token_out = es.convert( quantity,  itr->quote.balance.symbol );
            });
            */
            eosiosystem::exchange_state cont;
            cont.key = itr->key;
            cont.supply = itr->supply;
            cont.base.contract = itr->base.contract;
            cont.base.balance = itr->base.balance;
            cont.base.weight = itr->base.weight;
            
            cont.quote.contract = itr->quote.contract;
            cont.quote.balance = itr->quote.balance;
            cont.quote.weight = itr->quote.weight;
            token_out = cont.convert( quantity,  itr->quote.balance.symbol );            
        }
    }
    token_out.print();
    eosio_assert( token_out.amount > 0, "token amount received from selling EOS is too low" );
    std::string msg;
    msg = std::to_string(token_out.amount);
    
    msg = msg + '|';
    
    msg = msg + std::to_string(token_out.symbol.precision());
    
    std::string out_symbol;    
    tic_symbol_to_string(token_out.symbol, out_symbol);
    
    msg = msg + '|';
    msg = msg + out_symbol;
    msg = msg + '|';
    
    if_error_return(false, 9, msg); 
}

void token::setconfig(account_name admin, asset quote_quantity, uint64_t cw, string symbol)
{
    eosio_assert( cw > 0 && cw < 1000, "cw must in (0,1000)");

    auto symq = quote_quantity.symbol;
    eosio_assert( symq.is_valid(), "quote_quantity invalid symbol name" );    
    eosio_assert( quote_quantity.is_valid(), "invalid quote_quantity" );    
    
    uint64_t symboluint;
    symboluint = ::eosio::string_to_symbol(4,symbol.c_str());
   
    symbol_type token_symbol(symboluint);
    token_symbol.print();
    
    eosio_assert(token_symbol.is_valid(), "invalid token_symbol...");

    uint64_t key = token_symbol.value;
    
    markets _market(_self,_self);

    auto itr = _market.find(key);

    eosio_assert(itr != _market.end(), "token does not exist......");
    
    eosio_assert(itr->base.weight < 1000, "weight is 1000,can not modify");
    
    require_auth( itr->limit_admin );
    
    eosio_assert( quote_quantity.symbol == itr->quote.balance.symbol, "quote_quantity mismatch" );

    if(quote_quantity.amount > 0)
    {
        /*填充预发行储备金池*/
        action(
                permission_level{ itr->limit_admin, N(active) },
                itr->quote.contract, N(transfer),
                std::make_tuple(itr->limit_admin, _self, quote_quantity, std::string("initial.setconfig"))
        ).send();
    }
    
    _market.modify( itr, 0, [&]( auto& es ) {
        es.base.weight = cw;   
        es.quote.weight = cw;
        es.quote.balance += quote_quantity;
        es.limit_admin = admin;
    });
}

void token::regadmwhite( account_name admin, account_name newadmin )
{
    auto admin_existing = whitelists_table.find(admin);
    if(admin_existing != whitelists_table.end())
    {
        require_auth( admin_existing->contract );
        eosio_assert(admin_existing->isadmin == true, "is not whitelist admin.");
        whitelists_table.modify(admin_existing,_self, [&]( auto& s ) {
            s.contract = newadmin;
        });
    }
    else 
    {
        require_auth( _self );
        whitelists_table.emplace( _self, [&]( auto& s ) {
            s.contract = admin;
            s.enabled = true;
            s.created = false;
            s.isadmin = true;
        });
    }
}

void token::removewhite(account_name contract)
{
    auto admin_existing = whitelists_table.find(contract);
    eosio_assert(admin_existing != whitelists_table.end(), "admin does not exist");

    require_auth( admin_existing->contract );
    eosio_assert(admin_existing->isadmin == true, "is not whitelist admin.");
   
    eosio::multi_index<N(whitelist), whitelist>::const_iterator ite = whitelists_table.begin();
    while(ite != whitelists_table.end()) 
    {
        ite = whitelists_table.erase(ite);
    }
}

void token::regwhitelist( account_name admin, account_name contract, bool enabled )
{
    auto admin_existing = whitelists_table.find(admin);
    eosio_assert(admin_existing != whitelists_table.end(), "admin does not exist");

    require_auth( admin_existing->contract );
    eosio_assert(admin_existing->isadmin == true, "is not whitelist admin.");
    
    auto existing = whitelists_table.find(contract);
    if(existing != whitelists_table.end())
    {
        whitelists_table.modify(existing,_self, [&]( auto& s ) {
            s.contract = contract;
            s.enabled = enabled;
        });
    }
    else 
    {
        whitelists_table.emplace( _self, [&]( auto& s ) {
            s.contract = contract;
            s.enabled = enabled;
            s.created = false;
            s.isadmin = false;
        });
    }
}

void token::close( account_name owner, string symbol )
{
   require_auth( owner );
   
   uint64_t symboluint;
   /*string类型的代币名称转换为uint64类型*/
   symboluint = ::eosio::string_to_symbol(4,symbol.c_str());
   
   /*通过uint64类型的代币名称初始化symbol_type类型*/
   symbol_type token_symbol(symboluint);
    
   accounts acnts( _self, owner );
   auto it = acnts.find( token_symbol.name() );
   eosio_assert( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
   eosio_assert( it->balance.amount == 0, "Cannot close because the balance is not zero." );
   acnts.erase( it );
}

void token::on( const currency::transfer& t, account_name code ) 
{
    if( t.from == _self )
    {
        /*忽略_self transfer其他token的通知*/
        return;
    }
    if( t.to != _self ) 
    {
        /*防止"假转账通知"攻击*/
        return;
    }

    eosio_assert(t.quantity.amount > 0, "must purchase a positive amount" );

    if(t.memo.substr(0,7) == "initial" || t.memo.empty())
    {
        print("initial..\n");
        return;
    }
    
    /*string类型的代币名称转换为symbol_type类型*/
    uint64_t symboluint;
    symboluint = ::eosio::string_to_symbol(4,t.memo.c_str());
   
    symbol_type token_symbol(symboluint);
    
    eosio_assert(token_symbol.is_valid(), "invalid token_symbol....");

    uint64_t key = token_symbol.value;
    
    markets _market(_self,_self);

    auto itr = _market.find(key);

    eosio_assert(itr != _market.end(), "token does not exist.......");
 
    eosio_assert(itr->quote.balance.symbol.value == t.quantity.symbol.value, "buy:quote.balance.symbol mismatching..");
    
    asset token_out(0,token_symbol);
    if(itr->base.weight == 1000)
    {
        /*1:1兑换*/           
        token_out.amount = t.quantity.amount;
        in_issue(token_out, "issue by _self");
        
        _market.modify( itr, 0, [&]( auto& es ) {           
            es.base.balance += token_out;
            es.quote.balance += t.quantity;            
        });                   
    }
    else
    {
        _market.modify( itr, 0, [&]( auto& es ) {
            token_out = es.convert( t.quantity,  token_symbol );
        }); 
    }

    eosio_assert( token_out.amount > 0, "must reserve a positive amount" );

    /*发放用户买的token(为了收到Token转账通知,改为调用action)*/    
    action(
            permission_level{ itr->base.contract, N(active) },
            itr->base.contract, N(transfer),
            std::make_tuple(itr->base.contract, t.from, token_out, t.memo)
    ).send();
}

} /// namespace eosio

void eosio::token::apply( account_name contract, account_name act ) 
{
  /*收到其他代币合约的通知,表示是用其他Token来buy智能代币*/
  if( act == N(transfer) && contract != _self ) 
  {
     on( unpack_action_data<currency::transfer>(), contract );
     return;
  }
  
  /*防止"假转账通知"攻击*/
  if( contract != _self )
     return;
     
  auto& thiscontract = *this;
  switch (act) { EOSIO_API(eosio::token, (create)(issue)(retire)(destroy)(transfer)(setconfig)(close)(regwhitelist)(regadmwhite)(precalculate)); };
}

extern "C" {
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) 
    {
        eosio::token tk( receiver );
        tk.apply( code, action );
        /*eosio_exit(0);*/
    }
}
