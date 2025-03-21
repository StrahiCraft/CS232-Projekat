#include "ProgressBar.h"

ProgressBar::ProgressBar() {

}

ProgressBar::ProgressBar(glm::vec2 position, glm::vec2 scale,
	glm::vec3 progressBarColor, glm::vec3 backgroundColor, glm::vec3 fastDecayColor) {
	setPosition(position);
	setScale(scale);
	_progressBarColor = progressBarColor;
	_backgroundColor = backgroundColor;
	_fastDecayColor = fastDecayColor;
	_progress = 0;
	_fastDecayStart = 0;
	_fastDecayProgress = 0;
	_fastDecayGoal = 0;
	_fastDecayTimer = 1;
}

void ProgressBar::update() {
	if (_fastDecayTimer >= 1) {
		_fastDecayTimer = 1;
		_fastDecayProgress = 0;
		return;
	}

	_fastDecayTimer += Time::getRealTime();
	_fastDecayProgress = _fastDecayStart + (_fastDecayGoal - _fastDecayStart) * _fastDecayTimer;
}

void ProgressBar::updateProgressBar(float newProgress) {
	_progress = newProgress;
}

void ProgressBar::render() {
	if (!_active) {
		return;
	}
	glPushMatrix();

	glTranslatef(_position.x, _position.y, 0.0f);
	glRotatef(_rotation, 0.0f, 0.0f, 1.0f);
	glScalef(_scale.x, _scale.y, 1.0f);

	glBegin(GL_QUADS);

	drawProgressLayer(_maxProgress, _backgroundColor);
	drawProgressLayer(_fastDecayProgress, _fastDecayColor);
	drawProgressLayer(_progress, _progressBarColor);

	glEnd();

	glPopMatrix();
}

void ProgressBar::drawProgressLayer(float multiplier, glm::vec3 color) {
	glColor3f(color.x, color.y, color.z);
	glVertex2f(-0.5f, 0.5f);
	glVertex2f(-0.5f + multiplier, 0.5f);
	glVertex2f(-0.5f + multiplier, -0.5f);
	glVertex2f(-0.5f, -0.5f);
}

void ProgressBar::decayProgressBar(float newProgress) {
	_fastDecayStart = _progress;
	_progress = newProgress;
	_fastDecayGoal = _progress;
	_fastDecayTimer = 0;
}

float ProgressBar::getProgress()
{
	return _progress;
}

void ProgressBar::setProgress(float progress) {
	_progress = progress;
	if (_progress > _maxProgress) {
		_progress = _maxProgress;
	}
}

void ProgressBar::changeDecayColor(glm::vec3 newColor) {
	_fastDecayColor = newColor;
}

void ProgressBar::changeColor(glm::vec3 newColor) {
	_progressBarColor = newColor;
}

void ProgressBar::setMaxProgress(float value) {
	_maxProgress = value;

	if (_maxProgress > 1) {
		_maxProgress = 1;
	}

	if (_progress > _maxProgress) {
		_progress = _maxProgress;
	}

	if (_fastDecayProgress > _maxProgress) {
		_fastDecayProgress = _progress;
	}
}

float ProgressBar::getMaxProgress() {
	return _maxProgress;
}
