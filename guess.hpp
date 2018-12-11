/**
 *  @file
 *  @copyright create by yucheng 11-23-2018
 */
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <stdlib.h>
#include <stdio.h>

using namespace eosio;
using namespace std;

class guess : public eosio::contract
{
  public:
    using contract::contract;

    guess(account_name self) : contract(self),
                                  accounts(_self, _self),
                                  bets(_self, _self),
                                  gbet(_self, _self)
    {
    }

  public:
    // @abi action
    void hi(string name);

    // @abi action
    void rand();

    // @abi action
    void deposit(account_name from, asset &qty);

    // @abi action
    void openbet();

    // @abi action
    void reveal(const asset &bet, account_name player, const uint64_t betOn);

    // @abi action
    void reset();

    uint32_t _getrandnum1();

    uint32_t _getrandnum2();

    uint32_t _getrandnum(uint32_t num);

  private:
    //@abi table account i64
    struct account
    {
        account_name owner;
        asset balance;

        bool is_empty() const { return balance.amount; }

        uint64_t primary_key() const { return owner; }

        EOSLIB_SERIALIZE(account, (owner)(balance));
    };

    typedef eosio::multi_index<N(account), account> account_index;
    account_index accounts;

    //@abi table gbet i64
    struct gbet
    {
        uint64_t id;
        uint64_t betid;
        uint64_t open = 0;
        uint64_t reveal = 0;

        uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(gbet, (id)(betid)(open)(reveal))
    };

    typedef eosio::multi_index<N(gbet), gbet> gbet_index;
    gbet_index gbet;

    //@abi table bet i64
    struct bet
    {
        uint64_t id;
        account_name owner;
        asset balance;
        uint64_t betid;
        uint64_t p1;
        uint64_t p2;
        uint64_t p3;
        uint64_t p4;
        uint64_t p5;
        uint64_t p6;
        uint64_t betOn;
        uint64_t winner;

        uint64_t primary_key() const { return id; }

        uint64_t by_betid() const { return betid; }

        account_name by_owner() const { return owner; }

        EOSLIB_SERIALIZE(bet, (id)(owner)(balance)(betid)(p1)(p2)(p3)(p4)(p5)(p6)(betOn)(winner))
    };

    typedef eosio::multi_index<N(bet), bet,
                               indexed_by<N(bybetid), const_mem_fun<bet, account_name, &bet::by_owner>>>
        bet_index;
    bet_index bets;
};
