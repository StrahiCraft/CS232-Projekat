#include "GameState.h"
#include "ProgressBar.h"
#include "Ring.h"
#include "ChaosControl.h"
#include "Hitbox.h"
class Fishing :
    public GameState
{
private:
    ProgressBar* _progressBar;
    float _maxFishingTime;
    float _remainingFishingTime;
    Pickup* _pickup;
    Dictionary<glm::vec2, Hitbox*> _stoneHitboxes;
    Dictionary<glm::vec2, Sprite*> _stoneSprites;
public:
    Fishing(Player* player, Fish* fish) : GameState(player, fish){}
    void onStateEnter();
    void onStateUpdate();
    void onStateExit();
    void render();
    float getRemainigFishingTime();
    Dictionary<glm::vec2, Sprite*> getStoneSprites();
private:
    void resetPickup();
};