#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/symbol.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("sicbo")]] sicbo : public eosio::contract {
public:
	sicbo(name receiver, name code, datastream<const char *> ds):
		contract(receiver, code, ds), _gambling_table(receiver, code.value),
		_stake_table0(receiver, "stakeone"_n.value), _stake_table1(receiver, "staketwo"_n.value)
		{}

	// action list
	[[eosio::action]] void transfer(name from, name to, asset amount, string memo);
	[[eosio::action]] void dice(uint8_t face);
	[[eosio::action]] void nextround();
	[[eosio::action]] void start();
	[[eosio::action]] void stop();
	[[eosio::action]] void pause();
	[[eosio::action]] void resume();

private:
    void aaaa(name player, asset quantity,vector<uint8_t> playload);
    void checkSymbolCode(const asset& quantity);
    int checkAndChange(string item);
    void payForInviter();
    bool isNum(string str);
    void split(const string& memo,vector<string>& playload,const char flat);
    void init_stake();

	struct [[eosio::table]] stake {
		symbol  _sym;
		name    _player;
		vector<uint16_t> _stake;

		uint64_t primary_key() const { return _player.value; }
	};

	struct [[eosio::table]] gambling {
		uint64_t key;
		uint64_t value;

		uint64_t primary_key() const { return key; }
	};

	typedef eosio::multi_index<"stake"_n, stake> stake_table;
	typedef eosio::multi_index<"gambling"_n, gambling> gambling_table;

	// local variable for multi indexs
	stake_table _stake_table0;
	stake_table _stake_table1;
	gambling_table _gambling_table;

	// common function
	void init(uint64_t key, uint64_t value);
	void _pause(bool flag);
};

EOSIO_DISPATCH(sicbo, (transfer)(dice)(nextround)(start)(stop)(pause)(resume))
