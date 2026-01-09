


struct Input {
    uint32_t playerId;
    Direction dir;
};

class Controller {
public:
    virtual ~Controller() = default;
    virtual std::optional<Direction> update(Game& game) = 0;
};
