#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
using namespace std;
using namespace this_thread;
using namespace chrono;

random_device engine;
mt19937 mt(engine());


struct Card {
	Card(int number, int suit) :rank(number), suit(suit) {}

	string toString() const {
		string ranks[] = { "A","2","3","4","5","6","7","8","9","10","J","Q","K" };
		string suits[] = { "ハート","ダイヤ","クラブ","スペード" };
		return suits[suit] + "の" + ranks[rank];
	}

	int getValue() const {
		if (rank == 0) {
			return 11; // Aはひとまず11とする
		}
		else if (rank >= 10) {
			return 10; // J,Q,Kは10
		}
		else {
			return rank + 1;
		}
	}

private:
	int rank;
	int suit;
};

struct Deck {
	void shuffleDeck() {
		shuffle(cards.begin(), cards.end(), mt);
	}

	Deck() {
		for (int i = 0;i < 4;i++) {
			for (int j = 0;j < 13;j++) {
				cards.push_back(Card(j, i));
			}
		}
		shuffleDeck();
	}



	Card drawCard() {
		if (cards.empty()) {
			throw runtime_error("デッキが空です。ゲームを中止します。");
		}
		Card drawnCard = cards[0];
		cards.erase(cards.begin());
		return drawnCard;
	}

private:
	vector<Card> cards;
};

struct Player {
	Player(const string& name, int startingChips) :name(name), score(0), chips(startingChips) {}

	void addCard(const Card& card) {
		hand.push_back(card);
		if (card.getValue() == 11) {
			AceNum += 1;
		}
		score += card.getValue();
		while (score > 21) {
			if (AceNum == 0) {
				break;
			}
			else {
				score -= 10;
				AceNum -= 1;
			}
		}

		if (hand.size() == 2 && score == 21) {
			hasBlackjack = true;
		}
		else {
			hasBlackjack = false;
		}
	}

	string getName() const {
		return name;
	}

	int getScore() const {
		return score;
	}

	int getChips() const {
		return chips;
	}

	const vector<Card>& getHand() const {
		return hand;
	}

	void clearHand() {
		hand.clear();
		score = 0;
		hasBlackjack = false;
		AceNum = 0;
	}

	void addChips(int addAmount) {
		chips += addAmount;
	}

	void removeChips() {
		chips -= betAmount;
	}

	int betChips() {
		cout << name << "のベット額（現在の持ち点：" << chips << "）:";
		cin >> betAmount;

		while (betAmount<1 || betAmount>chips) {
			cout << "ベット額が適切ではありません。1以上" << chips << "以下で入力してください:";
			cin >> betAmount;
		}

		return betAmount;
	}

	void displayHand() const {
		cout << name << "の手札:";
		for (const auto& card : hand) {
			cout << card.toString() << " ";
		}
		cout << "（合計:" << score << "）" << endl;
	}

	void setBetAmount(int amount) {
		betAmount = amount;
	}

	int getBetAmount() const {
		return betAmount;
	}

	bool isBroke() {
		return chips <= 0 && !onGame;
	}

	void inGame(bool isGame) {
		onGame = false;
		if (isGame) {
			onGame = true;
		}
	}

	bool isBlackjack() {
		return hasBlackjack;
	}

private:
	string name;
	vector<Card> hand;
	int score;
	int chips;
	int betAmount;
	bool onGame = false;
	bool hasBlackjack = false;
	int AceNum = 0;
};


void drawingCard(vector<Player>& players, Deck& deck) {
	for (int i = 0;i < players.size();i++) {
		if (players[i].isBroke()) {
			continue;
		}
		players[i].addCard(deck.drawCard());
		players[i].displayHand();
	}
	sleep_for(seconds(1));
}

int main() {

	cout << "ブラックジャックへようこそ！" << endl;

	vector<int> defaltSettings;
	int numPlayers;
	cout << "プレイヤーの人数を入力してください:";
	cin >> numPlayers;

	int startChips;
	cout << "最初の持ち点を入力してください:";
	cin >> startChips;


	vector<Player> players;
	for (int i = 0;i < numPlayers;i++) {
		string playerName;
		cout << "プレイヤー" << i + 1 << "の名前を入力してください:";
		cin >> playerName;
		players.push_back(Player(playerName, startChips));
	}

	bool allPlayersBroke = false;

	while (true) {
		try {
			Deck deck;
			for (int i = 0;i < players.size();i++) {
				if (players[i].isBroke()) {
					continue;
				}
				
				players[i].inGame(true);

				players[i].clearHand();
				cout << players[i].getName() << "の持ち点: " << players[i].getChips() << endl;

				int betAmount = players[i].betChips();
				players[i].setBetAmount(betAmount);
				players[i].removeChips();
			}



			drawingCard(players, deck);

			sleep_for(seconds(1));


			int dealerScore = 0;
			int dealerAceNum = 0;
			Card dealerFaceUpCard = deck.drawCard(); //ディーラーの公開カード
			dealerScore += dealerFaceUpCard.getValue();
			if (dealerFaceUpCard.getValue() == 11) {
				dealerAceNum += 1;
			}
			cout << "ディーラーの公開カード:" << dealerFaceUpCard.toString() << endl;

			drawingCard(players, deck);
			cout << endl;

			sleep_for(seconds(1));

			Card dealerFaceDownCard = deck.drawCard(); //ディーラーの非公開カード

			for (int i = 0; i < players.size(); i++) {
				if (players[i].isBroke()) {
					continue;
				}

				if (players[i].isBlackjack()) {
					cout << players[i].getName() << "がブラックジャックを達成しました！" << endl;
				}
			}

			for (int i = 0;i < players.size();i++) { //プレイヤーの選択
				if (players[i].isBroke()) {
					continue;
				}

				while (players[i].getScore() <= 21) {
					char choice;
					cout << players[i].getName() << "のターン。現在のスコア:" << players[i].getScore() << "。ヒット（h）する？またはスタンドする（s）？";
					cin >> choice;

					if (choice == 'h') {
						players[i].addCard(deck.drawCard());
						players[i].displayHand();
					}
					else if (choice == 's') {
						break;
					}
					else {
						cout << "無効な選択です。 'h' または 's' を入力してください。" << endl;
					}
				}

				if (players[i].getScore() > 21) {
					cout << players[i].getName() << "はバーストした！" << endl << endl;
					continue;
				}
				cout << players[i].getName() << "のスコア:" << players[i].getScore() << endl << endl;
			}

			bool allPlayerBurst = true;
			for (int i = 0;i < players.size();i++) {
				if (players[i].isBroke()) {
					continue;
				}
				if (players[i].getScore() > 21) {
					continue;
				}
				allPlayerBurst = false;
			}

			sleep_for(seconds(1));

			cout << "ディーラーの非公開カード:" << dealerFaceDownCard.toString() << endl;
			dealerScore += dealerFaceDownCard.getValue();
			if (dealerFaceDownCard.getValue() == 11) {
				dealerAceNum += 1;
			}
			cout << "ディーラーのスコア:" << dealerScore << endl;
			bool isDealerBlackjack = false;
			if (dealerScore == 21) {
				isDealerBlackjack = true;
				cout << "ディーラーはブラックジャックです！" << endl;
			}
			bool isDealerBurst = false;
			sleep_for(seconds(1));

			if (allPlayerBurst == false) {
				while (dealerScore < 17) {
					Card dealrHitCard = deck.drawCard();
					cout << "ディーラーの引いたカード:" << dealrHitCard.toString() << endl;
					dealerScore += dealrHitCard.getValue();
					if (dealrHitCard.getValue() == 11) {
						dealerAceNum += 1;
					}
					if (dealerScore > 21 && dealerAceNum >0) {
						dealerScore -= 10;
						dealerAceNum -= 1;
					}
					cout << "ディーラーのスコア:" << dealerScore << endl;
					sleep_for(seconds(1));
				}



				if (dealerScore > 21) {
					isDealerBurst = true;
					cout << "ディーラーはバーストした！" << endl;
				}
			}
			cout << endl;

			sleep_for(seconds(1));

			for (int i = 0;i < players.size();i++) {
				if (players[i].isBroke()) {
					continue;
				}

				players[i].displayHand();

				if (players[i].getScore() > 21 || (isDealerBurst == false && dealerScore > players[i].getScore()) || (players[i].getScore() == dealerScore && isDealerBlackjack && !players[i].isBlackjack())) {
					cout << players[i].getName() << "の負けです！" << endl;
				}
				else if (isDealerBurst || players[i].getScore() > dealerScore || (players[i].getScore() == dealerScore && players[i].isBlackjack() && !isDealerBlackjack)) {
					if (players[i].isBlackjack()) {
						cout << players[i].getName() << "はブラックジャックを達成しています！" << endl;
						players[i].addChips(ceil(2.5 * players[i].getBetAmount()));
					}
					else {
						cout << players[i].getName() << "の勝ちです！" << endl;
						players[i].addChips(2 * players[i].getBetAmount());
					}
				}
				else {
					cout << players[i].getName() << "とディーラーの引き分けです！" << endl;
					players[i].addChips(players[i].getBetAmount());
				}

				players[i].inGame(false);
			}

			allPlayersBroke = true;
			for (int i = 0;i < players.size();i++) {
				if (players[i].isBroke()) {
					cout << players[i].getName() << "の持ち点がなくなりました！" << endl;
					sleep_for(seconds(1));
					continue;
				}
				allPlayersBroke = false;
			}

			if (allPlayersBroke) {
				cout << "全てのプレイヤーの持ち点がなくなりました。ゲームを終了します。";
				break;
			}

			char continueChoice;
			cout << "続けますか？（y/n）:";
			cin >> continueChoice;

			while (continueChoice != 'y' && continueChoice != 'n') {
				cout << "無効な選択です。 'y' または 'n' を入力してください:";
				cin >> continueChoice;
			}

			if (continueChoice == 'n') {
				for (int i = 0;i < players.size();i++) {
					cout << players[i].getName() << "の最終持ち点: " << players[i].getChips() << endl;
				}
				break;
			}


		}
		catch (const runtime_error& e) {
			cout << "エラーが発生しました: " << e.what() << endl;
			break;
		}
	}
}