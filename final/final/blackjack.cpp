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
		string suits[] = { "�n�[�g","�_�C��","�N���u","�X�y�[�h" };
		return suits[suit] + "��" + ranks[rank];
	}

	int getValue() const {
		if (rank == 0) {
			return 11; // A�͂ЂƂ܂�11�Ƃ���
		}
		else if (rank >= 10) {
			return 10; // J,Q,K��10
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
			throw runtime_error("�f�b�L����ł��B�Q�[���𒆎~���܂��B");
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
		cout << name << "�̃x�b�g�z�i���݂̎����_�F" << chips << "�j:";
		cin >> betAmount;

		while (betAmount<1 || betAmount>chips) {
			cout << "�x�b�g�z���K�؂ł͂���܂���B1�ȏ�" << chips << "�ȉ��œ��͂��Ă�������:";
			cin >> betAmount;
		}

		return betAmount;
	}

	void displayHand() const {
		cout << name << "�̎�D:";
		for (const auto& card : hand) {
			cout << card.toString() << " ";
		}
		cout << "�i���v:" << score << "�j" << endl;
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

	cout << "�u���b�N�W���b�N�ւ悤�����I" << endl;

	vector<int> defaltSettings;
	int numPlayers;
	cout << "�v���C���[�̐l������͂��Ă�������:";
	cin >> numPlayers;

	int startChips;
	cout << "�ŏ��̎����_����͂��Ă�������:";
	cin >> startChips;


	vector<Player> players;
	for (int i = 0;i < numPlayers;i++) {
		string playerName;
		cout << "�v���C���[" << i + 1 << "�̖��O����͂��Ă�������:";
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
				cout << players[i].getName() << "�̎����_: " << players[i].getChips() << endl;

				int betAmount = players[i].betChips();
				players[i].setBetAmount(betAmount);
				players[i].removeChips();
			}



			drawingCard(players, deck);

			sleep_for(seconds(1));


			int dealerScore = 0;
			int dealerAceNum = 0;
			Card dealerFaceUpCard = deck.drawCard(); //�f�B�[���[�̌��J�J�[�h
			dealerScore += dealerFaceUpCard.getValue();
			if (dealerFaceUpCard.getValue() == 11) {
				dealerAceNum += 1;
			}
			cout << "�f�B�[���[�̌��J�J�[�h:" << dealerFaceUpCard.toString() << endl;

			drawingCard(players, deck);
			cout << endl;

			sleep_for(seconds(1));

			Card dealerFaceDownCard = deck.drawCard(); //�f�B�[���[�̔���J�J�[�h

			for (int i = 0; i < players.size(); i++) {
				if (players[i].isBroke()) {
					continue;
				}

				if (players[i].isBlackjack()) {
					cout << players[i].getName() << "���u���b�N�W���b�N��B�����܂����I" << endl;
				}
			}

			for (int i = 0;i < players.size();i++) { //�v���C���[�̑I��
				if (players[i].isBroke()) {
					continue;
				}

				while (players[i].getScore() <= 21) {
					char choice;
					cout << players[i].getName() << "�̃^�[���B���݂̃X�R�A:" << players[i].getScore() << "�B�q�b�g�ih�j����H�܂��̓X�^���h����is�j�H";
					cin >> choice;

					if (choice == 'h') {
						players[i].addCard(deck.drawCard());
						players[i].displayHand();
					}
					else if (choice == 's') {
						break;
					}
					else {
						cout << "�����ȑI���ł��B 'h' �܂��� 's' ����͂��Ă��������B" << endl;
					}
				}

				if (players[i].getScore() > 21) {
					cout << players[i].getName() << "�̓o�[�X�g�����I" << endl << endl;
					continue;
				}
				cout << players[i].getName() << "�̃X�R�A:" << players[i].getScore() << endl << endl;
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

			cout << "�f�B�[���[�̔���J�J�[�h:" << dealerFaceDownCard.toString() << endl;
			dealerScore += dealerFaceDownCard.getValue();
			if (dealerFaceDownCard.getValue() == 11) {
				dealerAceNum += 1;
			}
			cout << "�f�B�[���[�̃X�R�A:" << dealerScore << endl;
			bool isDealerBlackjack = false;
			if (dealerScore == 21) {
				isDealerBlackjack = true;
				cout << "�f�B�[���[�̓u���b�N�W���b�N�ł��I" << endl;
			}
			bool isDealerBurst = false;
			sleep_for(seconds(1));

			if (allPlayerBurst == false) {
				while (dealerScore < 17) {
					Card dealrHitCard = deck.drawCard();
					cout << "�f�B�[���[�̈������J�[�h:" << dealrHitCard.toString() << endl;
					dealerScore += dealrHitCard.getValue();
					if (dealrHitCard.getValue() == 11) {
						dealerAceNum += 1;
					}
					if (dealerScore > 21 && dealerAceNum >0) {
						dealerScore -= 10;
						dealerAceNum -= 1;
					}
					cout << "�f�B�[���[�̃X�R�A:" << dealerScore << endl;
					sleep_for(seconds(1));
				}



				if (dealerScore > 21) {
					isDealerBurst = true;
					cout << "�f�B�[���[�̓o�[�X�g�����I" << endl;
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
					cout << players[i].getName() << "�̕����ł��I" << endl;
				}
				else if (isDealerBurst || players[i].getScore() > dealerScore || (players[i].getScore() == dealerScore && players[i].isBlackjack() && !isDealerBlackjack)) {
					if (players[i].isBlackjack()) {
						cout << players[i].getName() << "�̓u���b�N�W���b�N��B�����Ă��܂��I" << endl;
						players[i].addChips(ceil(2.5 * players[i].getBetAmount()));
					}
					else {
						cout << players[i].getName() << "�̏����ł��I" << endl;
						players[i].addChips(2 * players[i].getBetAmount());
					}
				}
				else {
					cout << players[i].getName() << "�ƃf�B�[���[�̈��������ł��I" << endl;
					players[i].addChips(players[i].getBetAmount());
				}

				players[i].inGame(false);
			}

			allPlayersBroke = true;
			for (int i = 0;i < players.size();i++) {
				if (players[i].isBroke()) {
					cout << players[i].getName() << "�̎����_���Ȃ��Ȃ�܂����I" << endl;
					sleep_for(seconds(1));
					continue;
				}
				allPlayersBroke = false;
			}

			if (allPlayersBroke) {
				cout << "�S�Ẵv���C���[�̎����_���Ȃ��Ȃ�܂����B�Q�[�����I�����܂��B";
				break;
			}

			char continueChoice;
			cout << "�����܂����H�iy/n�j:";
			cin >> continueChoice;

			while (continueChoice != 'y' && continueChoice != 'n') {
				cout << "�����ȑI���ł��B 'y' �܂��� 'n' ����͂��Ă�������:";
				cin >> continueChoice;
			}

			if (continueChoice == 'n') {
				for (int i = 0;i < players.size();i++) {
					cout << players[i].getName() << "�̍ŏI�����_: " << players[i].getChips() << endl;
				}
				break;
			}


		}
		catch (const runtime_error& e) {
			cout << "�G���[���������܂���: " << e.what() << endl;
			break;
		}
	}
}