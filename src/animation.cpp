#include "animation.hpp"

Animation idleAnim;
Animation runAnim;
Animation skidAnim;
Animation jumpAnim;
Animation climbAnim;
Animation swimAnim;
Animation crouchAnim;

static AnimationFrame makeFrame(int tileIndex, int durationMs) {
    SDL_Rect rect = {
        (tileIndex - 1) * 48, 0, 48, 48
    };
    return { rect, durationMs };
}

void loadAnimations() {
    idleAnim.frames = { makeFrame(1, 1000) };
    idleAnim.loop = true;

    runAnim.frames = {
        makeFrame(2, 100),
        makeFrame(3, 100),
        makeFrame(4, 100)
    };
    runAnim.loop = true;

    skidAnim.frames = { makeFrame(5, 1000) };
    skidAnim.loop = false;

    jumpAnim.frames = { makeFrame(6, 1000) };
    jumpAnim.loop = false;

    climbAnim.frames = {
        makeFrame(7, 200),
        makeFrame(8, 200)
    };
    climbAnim.loop = true;

    swimAnim.frames = {
        makeFrame(9, 150),
        makeFrame(10, 150),
        makeFrame(11, 150),
        makeFrame(12, 150),
        makeFrame(13, 150)
    };
    swimAnim.loop = true;

    crouchAnim.frames = { makeFrame(15, 1000) };
    crouchAnim.loop = true;
}

void AnimationPlayer::play(const Animation* anim) {
    if (anim != current) {
        current = anim;
        frameIndex = 0;
        elapsed = 0;
    }
}

void AnimationPlayer::update(int delta, float speedMultiplier) {
    if (!current || current->frames.empty()) return;

    elapsed += static_cast<int>(delta * speedMultiplier);

    while (elapsed >= current->frames[frameIndex].duration) {
        elapsed -= current->frames[frameIndex].duration;
        frameIndex = (frameIndex + 1) % current->frames.size();
    }
}

SDL_Rect AnimationPlayer::currentFrame() const {
    if (!current || current->frames.empty())
        return SDL_Rect{0, 0, 0, 0};
    return current->frames[frameIndex].srcRect;
}
