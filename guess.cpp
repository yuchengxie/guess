/**
 *  @file
 *  @copyright create by yucheng 11-23-2018
 */

#include <guess.hpp>

void guess::hi(string name)
{
    print("welcome guess: ", name);
}

void guess::openbet()
{
    require_auth(_self);
    auto cur_gbet_itr = gbet.begin();
    if (cur_gbet_itr == gbet.end())
    {
        cur_gbet_itr = gbet.emplace(_self, [&](auto &g_bet) {
            g_bet.id = gbet.available_primary_key();
            g_bet.betid = 0;
            g_bet.open = 0;
            g_bet.reveal = 1;
        });
    }
    eosio_assert(cur_gbet_itr != gbet.end(), "bet is not build");
    eosio_assert(cur_gbet_itr->open == 0, "bet is opened");
    gbet.modify(cur_gbet_itr, 0, [&](auto &g_bet) {
        g_bet.betid++;
        g_bet.open = 1;
        g_bet.reveal = 0;

        auto new_bet_itr = bets.emplace(_self, [&](auto &rbet) {
            rbet.id = bets.available_primary_key();
            rbet.betid = g_bet.betid;
            rbet.winner = 0;
            rbet.p1 = _getrandnum(1);
            rbet.p2 = _getrandnum(2);
        });
    });
}

void guess::reveal(const asset &bet, account_name player, const uint64_t betOn)
{
    require_auth(_self);
    eosio_assert(bet.symbol == S(4, SYS), "token must be SYS");
    eosio_assert(bet.is_valid(), "invalid bet");
    eosio_assert(bet.amount > 0, "qty must greater than 0");

    auto cur_player_itr = accounts.find(player);
    eosio_assert(cur_player_itr != accounts.end(), "unknown account");
    eosio_assert(cur_player_itr->balance >= bet, "insufficient balance");

    auto cur_gbet_itr = gbet.begin();
    eosio_assert(cur_gbet_itr != gbet.end(), "bet is not build");
    eosio_assert(cur_gbet_itr->open == 1, "bet is not open");
    eosio_assert(cur_gbet_itr->reveal == 0, "bet is revealed");

    uint64_t betid = cur_gbet_itr->betid;
    uint64_t exist_bet = false;
    auto bybetid_index = bets.template get_index<N(bybetid)>();
    auto cur_bets_itr = bybetid_index.find(betid);
    while (cur_bets_itr != bybetid_index.end() && cur_bets_itr->betid == betid)
    {
        if (cur_bets_itr->owner == player)
        {
            exist_bet = true;
            break;
        }
        cur_bets_itr++;
    }
    eosio_assert(exist_bet == false, "bet is not exist");
    auto cur_bet_itr = bets.find(betid - 1);
    bets.modify(cur_bet_itr, 0, [&](auto &rbet) {
        auto p1 = rbet.p1;
        auto p2 = rbet.p2;
        rbet.balance = bet;
        rbet.owner = player;
        rbet.betOn = betOn;


        auto balance = bet;

        if (p1 == 0 || p2 == 0 || betOn == 0)
        {
            print("offerbet err");
            return;
        }
        //
        auto s1 = p1 ;
        auto s2 = p2 ;
        if ((s1 > s2 && betOn == 1) || (s1 < s2 && betOn == 2))
        {
            rbet.winner = 2;
            print("玩家胜利");
            //合约账号向玩家转账 TODO
            accounts.modify(cur_player_itr, 0, [&](auto &acnt) {
                acnt.balance += 2 * balance;
            });
        }
        else
        {
            //玩家转账给其他 TODO
            rbet.winner = 1;
            print("玩家失败");
            accounts.modify(cur_player_itr, 0, [&](auto &acnt) {
                acnt.balance -= bet;
            });
        }
    });

    gbet.modify(cur_gbet_itr, 0, [&](auto &g_bet) {
        g_bet.reveal = 1;
        g_bet.open = 0;
    });
}

void guess::reset()
{
    require_auth(_self);
    auto cur_gbet_itr = gbet.begin();
    while (cur_gbet_itr != gbet.end())
    {
        cur_gbet_itr = gbet.erase(cur_gbet_itr);
    }
    auto cur_bet_itr = bets.begin();
    while (cur_bet_itr != bets.end())
    {
        cur_bet_itr = bets.erase(cur_bet_itr);
    }
}

void guess::deposit(account_name from, asset &qty)
{
    require_auth(from);
    eosio_assert(qty.is_valid(), "invalid qty");
    eosio_assert(qty.amount > 0, "qty must greater than 0");
    auto itr = accounts.find(from);
    if (itr == accounts.end())
    {
        itr = accounts.emplace(_self, [&](auto &acnt) {
            acnt.owner = from;
        });
    }

    action(
        permission_level{from, N(active)},
        N(eosio.token), N(transfer),
        std::make_tuple(from, _self, qty, std::string(""))

            )
        .send();

    accounts.modify(itr, 0, [&](auto &acnt) {
        acnt.balance += qty;
    });
}

uint32_t guess::_getrandnum(uint32_t num)
{
    //1-3随机数
    return (((uint32_t)(current_time() * num - num)) % 3) + 1;
}

EOSIO_ABI(guess, (hi)(deposit)(openbet)(reset)(reveal))
