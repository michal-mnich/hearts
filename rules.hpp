

enum Seat {
    north = 'N',
    east = 'E',
    south = 'S',
    west = 'W'
};

class AbstractDealType {
public:
    virtual int calculatePenalty() = 0;
};

class NoTricks : AbstractDealType {

};

class NoHearts : AbstractDealType {

};

class NoQueens : AbstractDealType {

};

class NoJacksOrKings : AbstractDealType {

};

class NoKingOfHearts : AbstractDealType {

};

class NoSeventhOrLastTrick : AbstractDealType {

};

class Bandit : AbstractDealType {

};
