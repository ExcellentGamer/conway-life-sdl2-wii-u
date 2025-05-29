#pragma once

#include <vector>
#include <SDL.h>

// Animation frame structure
struct AnimationFrame {
    SDL_Rect srcRect;
    int duration; // milliseconds
};

// Full animation definition
struct Animation {
    std::vector<AnimationFrame> frames;
    bool loop = true;
};

// Animation player
class AnimationPlayer {
public:
    void play(const Animation* anim);
    void update(int delta, float speedMultiplier) ;
    SDL_Rect currentFrame() const;

private:
    const Animation* current = nullptr;
    int frameIndex = 0;
    int elapsed = 0;
};

// External animations (shared across the game)
extern Animation idleAnim;
extern Animation runAnim;
extern Animation skidAnim;
extern Animation jumpAnim;
extern Animation climbAnim;
extern Animation swimAnim;
extern Animation crouchAnim;

// Load all animations
void loadAnimations();
