#include "ReelingIn.h"

ReelingIn::ReelingIn(Player* player, Fish* fish, glm::vec2 windowSize, float lineIntegrity, float lineStrength,
	int bombCount, Dictionary<glm::vec2, Sprite*> stoneSprites) : GameState(player, fish)
{
	_windowSize = windowSize;
	_reelInThreshold = 60;
	_bombCount = bombCount;
	_player->setSpeedDebuff(fish->getWeight());
	_stoneSprites = stoneSprites;
	Time::setTimeScale(1);
	resetPickup();

	_progressBar = new ProgressBar(glm::vec2(400, 50), glm::vec2(500, 15),
		glm::vec3(1), glm::vec3(0.2f), glm::vec3(0.5f, 0, 0));
	_progressBar->updateProgressBar(lineIntegrity);
	_lineIntegrity = lineIntegrity;
	_progressBar->setMaxProgress(lineStrength);
}

void ReelingIn::onStateEnter()
{
	AudioManager::playSong("reelingIn");

	for (int i = 0; i < _bombCount; i++) {
		_bombs.push_back(new Bomb(glm::vec2(0), glm::vec2(0, -150),
			new Sprite("Sprites/bomb.png", glm::vec2(32), 1, glm::vec2(1), true), _windowSize));
		_bombs[i]->resetBomb();
		_bombs[i]->setScale(glm::vec2(2));
	}

	if (ScoreManager::getScore() < 30) {
		_reelInThreshold = 25;
		return;
	}
	if (ScoreManager::getScore() < 50) {
		_reelInThreshold = 45;
		return;
	}
	if (ScoreManager::getScore() < 100) {
		_reelInThreshold = 60;
		return;
	}
	_reelInThreshold = 90;
}

void ReelingIn::onStateUpdate()
{
	if (_lineBroken) {
		if (_lineBrokenTimer > 3) {
			EventSystem::invokeChannel("OnLineBreak");
			return;
		}
		_fish->setVelocity(glm::vec2(-200, -50));
		_fish->getSprite()->setSpriteFlip(glm::vec2(0, 1));
		_fish->setRotation(_fish->getVelocityAngle() + 90);
		_fish->setPosition(_fish->getPosition() + _fish->getVelocity() * Time::getDeltaTime());
		_lineBrokenTimer += Time::getDeltaTime();
		return;
	}

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

	if (_reelInScore >= _reelInThreshold) {
		EventSystem::invokeChannel("FishCaught");
		return;
	}

	if (_progressBar->getMaxProgress() <= 0) {
		EventSystem::invokeChannel("GameOver");
		return;
	}

	if (_lineIntegrity <= 0) {
		_lineBroken = true;
		AudioManager::stopMusic();
		AudioManager::playSound("lineBreak");
		return;
	}

	_stoneSpriteOffset += Time::getDeltaTime() * -150;
	_reelInScore += Time::getDeltaTime();

	BackgroundManager::setBackground(1 - (_reelInThreshold - _reelInScore) / 90.0f);

	if (_player->getVelocityMagnitude() == 0) {
		_lineIntegrity += Time::getDeltaTime() * _lineIntegrityDecaySpeed * ((_player->getPosition().y + 100) / 500);
		_progressBar->changeColor(glm::vec3(0.5f, 1, 0.5f));
		if (_lineIntegrity > _progressBar->getMaxProgress()) {
			_lineIntegrity = _progressBar->getMaxProgress();
		}
	}
	else {
		_lineIntegrity -= _player->getVelocityMagnitude() * Time::getDeltaTime() * _lineIntegrityDecaySpeed / ((_player->getPosition().y + 100) / 400);
		_progressBar->changeColor(glm::mix(glm::vec3(0.5f, 0, 0), glm::vec3(1), _lineIntegrity));
	}

	_progressBar->setProgress(_lineIntegrity);

	checkForBombCollision();

	_player->update();
	_progressBar->update();
	_fish->setPosition(_player->getPosition());
	_fish->setRotation(_player->getRotation() + 90);

	_pickup->update();

	if (glm::distance(_player->getPosition(), _pickup->getPosition()) <= _pickup->getSize() * _pickup->getScale().x) {
		_pickup->onPickup();
	}

	for (int i = 0; i < _bombCount; i++) {
		_bombs[i]->update();
	}

}

void ReelingIn::render()
{
	BackgroundManager::render();

	for (int i = 0; i < _stoneSprites.getItemCount(); i++) {
		glPushMatrix();
		glTranslatef(_stoneSprites.getKey(i).x * 64 - 32, _stoneSprites.getKey(i).y * 64 - 32 + _stoneSpriteOffset, 0);
		_stoneSprites.getValue(i)->render();
		glPopMatrix();
	}
	if (_lineBroken) {
		_player->renderBroken(_lineBrokenTimer);
	}
	else {
		_player->render();
	}
	_fish->render();

	for (int i = 0; i < _bombCount; i++) {
		_bombs[i]->render();
	}

	_pickup->render();

	BackgroundManager::renderOverlay();

	if (!_lineBroken) {
		_progressBar->render();
		ScoreManager::render();
	}

	if (_paused) {
		renderPauseMenu();
	}
}

void ReelingIn::clearBombs() {
	for (Bomb* bomb : _bombs) {
		bomb->resetBomb();
	}
}

float ReelingIn::getLineStrength() {
	return _progressBar->getMaxProgress();
}

void ReelingIn::onStateExit() {
	AudioManager::stopMusic();
}

void ReelingIn::checkForBombCollision() {
	for (Bomb* bomb : _bombs) {
		if (glm::distance(_player->getPosition(), bomb->getPosition()) <= 45) {
			AudioManager::playSound("explode");
			_progressBar->setMaxProgress(_progressBar->getMaxProgress() - 0.2f);

			if (_progressBar->getMaxProgress() <= 0) {
				EventSystem::invokeChannel("GameOver");
				return;
			}

			if (_lineIntegrity > _progressBar->getMaxProgress()) {
				_lineIntegrity = _progressBar->getMaxProgress();
			}
			if (_lineIntegrity >= 0.2f) {
				_lineIntegrity -= 0.1f;
				_progressBar->decayProgressBar(0.1f);
			}
			bomb->resetBomb();
		}
	}
}

void ReelingIn::resetPickup() {
	// change to the number of pickups intended for this state
	int pickupTypeCount = 4;

	switch (rand() % pickupTypeCount) {
	case 0:
		_pickup = new Ring(glm::vec2(50 + rand() % 700, 900), glm::vec2(0, -150));
		break;
	case 1:
		_pickup = new BurstStream(glm::vec2(50 + rand() % 700, 900), glm::vec2(0, -150));
		break;
	case 2:
		_pickup = new Ring(glm::vec2(50 + rand() % 700, 900), glm::vec2(0, -150), 2);
		break;
	case 3:
		_pickup = new Ring(glm::vec2(50 + rand() % 700, 900), glm::vec2(0, -150), 3);
		break;
	default:
		_pickup = new Ring(glm::vec2(50 + rand() % 700, 900), glm::vec2(0, -150));
		break;
	}
}