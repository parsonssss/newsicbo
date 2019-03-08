#include "bugxio.sicbo/bugxio.sicbo.hpp"
#include "bugxio.sicbo/globals.hpp"
#include <eosiolib/action.hpp>
#include <string>
#include <sstream>

#define MAX_BET_PRECENT 1.5

//游戏入口
//from -> 玩家
//to -> 合约
//quanity -> 金额以及token标志
//memo -> 下注信息
//->[双,单,小,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,大,inviter]
//检查数据,发放佣金,进行下注
//
//

void sicbo::transfer(name from,name to,asset quanity,string memo){
    //currency::transfer t = {from , to ,quanity , memo};
    if (from == _self || to != _self){
        return;
    }
    //检查游戏是否正在运行
    auto gameStatusItr = _gambling_table.find(GAMELOCK);
    if (gameStatusItr == _gambling_table.end()){
        _gambling_table.emplace(_self,[&](auto& info){
            info.key = GAMELOCK;
            info.value = false;
        });
    }
    eosio_assert(gameStatusItr->value == true,"Game is not stating now...");

    //检查玩家发过来的币种
    sicbo::checkSymbolCode(quanity);

    eosio_assert(quanity.is_valid(), "Invalid transfer amount.");

    //检查转账金额 > 0
    int64_t amt = quanity.amount;
    eosio_assert(amt > 0, "Transfer amount not positive");

    //判断memo是否为空
    eosio_assert(memo.empty() == false,"empty memo!");

    //获取赌注详情 与 邀请人
    vector<string> pieces;
    vector<uint8_t> playload;
    string inviter_str;

    if (memo.length() >= 1) {/*TODO cal the min length*/
	sicbo::split(memo,pieces,',');
        inviter_str = pieces[pieces.size()-1];
        pieces.pop_back();
    }
        //TODO reslove other memo

    //检查每个注是否为数字并验证下注金额与实际金额是否相同
    int64_t totalAmount = 0;
    for(auto each : pieces){
        uint8_t temp = sicbo::checkAndChange(each);
        eosio_assert(temp > 0,"Invalid memo message");
        totalAmount += temp;
	playload.push_back(temp);
    }
    eosio_assert(totalAmount == amt,"Invalid bet money");
    //TODO limit the min and the max range

    //TODO 记录每次下注金额,计算当前期数的下注总金额
    auto currentRound = _gambling_table.find(ROUND);
    bool whichBetsBool = currentRound->value % 2;
    auto whichBets = whichBetsBool ? BETS0 : BETS1;

    auto onceBetAmountItr = _gambling_table.find(whichBets);
	//TODO deal with nil
    uint64_t tempTotalAmount = onceBetAmountItr->value + totalAmount;
    eosio_assert(tempTotalAmount > SINGLE_BET_AMOUNT,"Bet Pool is over!");
    _gambling_table.modify(onceBetAmountItr,_self,[&](auto& a){
		    a.value = tempTotalAmount;
		    });


    //TODO 限制下注金额大小
    if (quanity.symbol == EOS_SYMBOL){
        //eosio::token bet_token(EOSIO_TOKEN);
    }else{
        if(quanity.symbol == BUX_SYMBOL){
        //eosio::token bet_token();
        }
        else{
            return;//TODO;
        }
    }
    //TODO 新版本重构
    //eosio::asset balance = bet_token.get_balance(_self,quanity.symbol.code)
    //float max_bet_token = (balance.amout * SINGLE_MAX_PRECENT_BET / 100)
    //eosio_assert(max_bet_token > quanity.amount,"Bet amount exceeds max amount")
    
    //获取并检验邀请人
    name inviter;
    if (inviter_str.empty() == true){
        inviter = TEAM_ACCOUNT;//TODO add self account
    }else{
        inviter = name(inviter_str);
        eosio_assert(is_account(inviter),"Invalid inviter");
        eosio_assert(inviter != from,"Can't Invite self");
    }


    sicbo::betStart(from,quanity,playload);
    //TODO 发送佣金(wrapper)

}

void sicbo::payForInviter(){
    require_auth(_self);
    return;
    //TODO
}

void sicbo::betStart(name from,asset quanity,vector<uint8_t> playload){
    require_auth(_self);
    auto betRound = _gambling_table.find(ROUND);
    bool betTableNum = (betRound->value % 2);
    auto& betTable = betTableNum ? _stake_table1 : _stake_table0;
    betTable.emplace(_self,[&](auto& info){
        info._player = from;
	for(auto p : playload){
		info._stake.push_back(uint8_t(p));
	}
        info._sym = quanity.symbol;
    });
}

void sicbo::checkSymbolCode(const eosio::asset& quantity) {
      eosio_assert(
      (_code == EOSIO_TOKEN && quantity.symbol == EOS_SYMBOL) ||
      (_code == BUGXIO_TOKEN && quantity.symbol == BUX_SYMBOL),
    "Token do not be supported, or symbol not match with the code!");
}

uint8_t sicbo::checkAndChange(string item){
    if (sicbo::isNum(item)){
        return atoi(item.c_str());
    }
    else{
        return -1;
    }
}

bool sicbo::isNum(string str)
{
    stringstream sin(str);
    double d;
    char c;
    if(!(sin >> d))
    {   
        return false;
    }
    if (sin >> c)
    {   
        return false;
    }
    return true;
}

void sicbo::split(const string& s,vector<string>& sv,const char flag = ',') {
    sv.clear();
    istringstream iss(s);
    string temp;

    while (getline(iss, temp, flag)) {
        sv.push_back(temp);
    }
    return;
}



//******************************************************************//

void sicbo::nextround() {
        require_auth(BUGXIO);

        bool new_season = false;

        auto itr = _gambling_table.find(ROUND);
        if (itr->value == MAX_UINT64) {
                new_season = true;
        }

        _gambling_table.modify(*itr, get_self(), [&](auto& p) {
                p.value += 1;
        });

        // 新一季游戏
        if (new_season) {
                _gambling_table.modify(*itr, get_self(), [&](auto& p) {
                        p.value += 1;
                });
        }
}

void sicbo::dice(uint8_t face) {
        require_auth(BUGXIO);

        auto itr = _gambling_table.find(ROUND);

        bool settle_this = (itr->value - 1) % 2;
        auto& this_table  = settle_this ? _stake_table1 : _stake_table0;
        auto this_bets   = settle_this ? BETS1 : BETS0;

        for (auto& s : this_table) {
                int64_t amount = 0;

                // 结算单双
                if (s._stake[face % 2] != 0) {
                        amount += ODDS[face % 2] * s._stake[face % 2];
                }

                // 结算大小
                bool big = face > 10 ? 19 : 2;
                if (s._stake[big] != 0) {
                        amount += ODDS[big] * s._stake[big];
                }

                // 结算点数
                if (s._stake[face] != 0) {
                        amount += ODDS[face] * s._stake[face];
                }
                if (amount != 0) {
                        const auto winbet = asset(amount, s._sym);
                        const auto memo = "Here is your winbet: ";// + winbet.to_string();

                        auto to = BUGXIO_TOKEN;
                        if (s._sym != BUX_SYMBOL) {
                                to = EOSIO_TOKEN;
                        }

                        action(
                        permission_level{get_self(), "active"_n},
                        to,
                        "transfer"_n,
                        make_tuple(get_self(), s._player, winbet, memo)
                        ).send();
                }

                this_table.erase(s);
        }

        init(this_bets, 0);
}

void sicbo::start() {
        require_auth(BUGXIO);

        init(VERSION,  SICBO_VERSION);
        init(ROUND,    0);
        init(SEASON,   1);
        init(BETS0,    0);
        init(BETS1,    0);
        init(GAMELOCK, 0);
	init_stake();
}

void sicbo::pause() {
        require_auth(BUGXIO);
        _pause(true);
}

void sicbo::resume() {
        require_auth(BUGXIO);
        _pause(false);
}

void sicbo::stop() {
        require_auth(BUGXIO);

	        //TODO
}

void sicbo::_pause(bool flag) {
        auto itr = _gambling_table.find(GAMELOCK);

        if (itr->value != flag) {
                _gambling_table.modify(*itr, get_self(), [&](auto& p) {
                        p.value = flag;
                });
        }
}

void sicbo::init(uint64_t key, uint64_t value) {
        auto itr = _gambling_table.find(key);

        if (itr == _gambling_table.end()) {
                _gambling_table.emplace(get_self(), [&](auto& p) {
                        p.key = key;
                        p.value = value;
                });
        }
}

void sicbo::init_stake() {
	uint16_t test = 1;
	vector<uint16_t> playload;
	playload.push_back(test);
	playload.push_back(test);
	playload.push_back(test);
	playload.push_back(test);
	playload.push_back(test);

	_stake_table1.emplace(get_self(), [&](auto& p) {
			p._sym = symbol("eos",4);
			p._player = "bob"_n;
			p._stake = playload;
	});

	_stake_table0.emplace(get_self(), [&](auto& p) {
			p._sym = symbol("eos",4);
			p._player = "alice"_n;
			p._stake = playload;
	});
        
}
