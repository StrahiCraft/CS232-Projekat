#include "Fishing.h"

Fishing::Fishing(Player* player, Fish* fish, float lineIntegrity) : GameState(player, fish) {
	_maxFishingTime = 30 * lineIntegrity;
	_remainingFishingTime = _maxFishingTime;
	_progressBar = new ProgressBar(glm::vec2(400, 50), glm::vec2(500, 15),
		glm::vec3(1), glm::vec3(0.2f), glm::vec3(0.5f, 0, 0));
	_progressBar->setMaxProgress(lineIntegrity);

	if (ScoreManager::getScore() < 30) {
		BackgroundManager::setBackground(1 - 25.0f / 90.0f);
		return;
	}
	if (ScoreManager::getScore() < 50) {
		BackgroundManager::setBackground(1 - 45.0f / 90.0f);
		return;
	}
	if (ScoreManager::getScore() < 100) {
		BackgroundManager::setBackground(1 - 60.0f / 90.0f);
		return;
	}
	BackgroundManager::setBackground(1 - 90.0f / 90.0f);
}

void Fishing::onStateEnter() {
	AudioManager::playSong("fishing");
	
	_player->setActive(true);
	_player->setRotation(0);
	_player->setVelocity(glm::vec2(0));
	_player->setPosition(glm::vec2(400, 400));
	_player->setSpeedDebuff(0);
	
	for (int i = 0; i < 5 + rand() % 25; i++) {
		glm::vec2 tilePosition = glm::vec2(rand() % 13, rand() % 13);
		_stoneHitboxes.addItem(tilePosition, new Hitbox(tilePosition * glm::vec2(64), glm::vec2(64), _player));
		_stoneSprites.addItem(tilePosition, new Sprite("Sprites/stone.png", glm::vec2(64), 1, glm::vec2(1), false));
		for (int y = 0; y < tilePosition.y; y++) {
			if (!_stoneSprites.containsKey(glm::vec2(tilePosition.x, y))) {
				_stoneSprites.addItem(glm::vec2(tilePosition.x, y), new Sprite("Sprites/stoneDark.png", glm::vec2(64), 1, glm::vec2(1), false));
			}
		}
	}

	resetPickup();
}

void Fishing::onStateUpdate() {

	// 27 = escape
	if (Input::getKeyDown(27)) {
		togglePause();
	}

	if (_paused) {
		return;
	}

	BackgroundManager::update();

	if (!_pickup->getActive()) {
		resetPickup();
	}

	if (glm::distance(_player->getPosition(), _fish->getPosition()) <= _fish->getGoalCircleSize() + 16) {
		_fish->increaseCaptureScore(1);
	}
	else {	
		_remainingFishingTime -= Time::getDeltaTime();
		_fish->increaseCaptureScore(-1);
	}

	_fish->update();
	_progressBar->updateProgressBar(_remainingFishingTime / _maxFishingTime * _progressBar->getMaxProgress());
	_progressBar->update();
	
	for (int i = 0; i < _stoneHitboxes.getItemCount(); i++) {
		_stoneHitboxes.getValue(i)->update();
	}

	_player->update();
	_pickup->update();

	if (glm::distance(_player->getPosition(), _pickup->getPosition()) <= _pickup->getSize() * _pickup->getScale().x) {
		_pickup->onPickup();
	}

	if (_remainingFishingTime < 0) {
		EventSystem::invokeChannel("GameOver");
	}
}

void Fishing::onStateExit() {
	AudioManager::stopMusic(); 
}

void Fishing::render() {
	BackgroundManager::render();

	for (int i = 0; i < _stoneSprites.getItemCount(); i++) {
		glPushMatrix();
		glTranslatef(_stoneSprites.getKey(i).x * 64 - 32, _stoneSprites.getKey(i).y * 64 - 32, 0);
		_stoneSprites.getValue(i)->render();
		glPopMatrix();
	}
	_fish->render();
	_player->render();
	_pickup->render();

	BackgroundManager::renderOverlay();

	_progressBar->render();
	ScoreManager::render();

	if (_paused) {
		renderPauseMenu();
	}
}

float Fishing::getRemainigFishingTime() {
	return _remainingFishingTime;
}

float Fishing::getLineStrength()
{
	return _progressBar->getMaxProgress();
}

Dictionary<glm::vec2, Sprite*> Fishing::getStoneSprites()
{
	return _stoneSprites;
}

void Fishing::resetPickup() {
	// change to the number of pickups intended for this state
	int pickupTypeCount = 3;

	switch (rand() % pickupTypeCount) {
	case 0:
		_pickup = new Ring(glm::vec2(-499 + rand() % 500, 50 + rand() % 700), glm::vec2(50 + rand() % 200, -1 + rand() % 2));
		break;
	case 1:
		_pickup = new ChaosControl(glm::vec2(-499 + rand() % 500, 50 + rand() % 700), glm::vec2(50 + rand() % 200, -1 + rand() % 2));
		break;
	case 2:
		_pickup = new Ring(glm::vec2(-499 + rand() % 500, 50 + rand() % 700), glm::vec2(50 + rand() % 200, -1 + rand() % 2), 2);
		break;
	default:
		_pickup = new Ring(glm::vec2(-499 + rand() % 500, 50 + rand() % 700), glm::vec2(50 + rand() % 200, -1 + rand() % 2));
		break;
	}
}
