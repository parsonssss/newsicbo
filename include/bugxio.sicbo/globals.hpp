#pragma once


#define SICBO_VERSION 1

//gambling infomation
#define ROUND    	0x1
#define SEASON   	0x2
#define GAMELOCK        0x3
#define VERSION  	0x4
#define BETS0	 	0x5
#define BETS1		0x6

// contract infomation
#define SINGLE_BET_AMOUNT 1
#define TEAM_ACCOUNT ("bugxio.token"_n)
#define MAX_UINT64   (0ULL - 1)
#define BUGXIO       ("bugxio"_n)
#define BUGXIO_TOKEN ("bugxio.token"_n)
#define EOSIO_TOKEN  ("eosio.token"_n)
#define EOS_SYMBOL   (symbol("EOS", 4))
#define BUX_SYMBOL   (symbol("BUX", 4))

// gambling odds
const uint8_t ODDS[20] = {
	2, 2, 2, // 双 单 小
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, //点数3-18
	2, //大
};
