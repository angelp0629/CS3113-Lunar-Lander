#ifndef ENTITY_H
#define ENTITY_H

#include "raylib.h"

// A simple game object following the structure of the CS3113 Entity lectures.
// Each Entity owns its texture, position, velocity, acceleration and collider.
class Entity
{
private:
    Vector2   mPosition;
    Vector2   mVelocity;
    Vector2   mAcceleration;
    Vector2   mScale;
    Vector2   mColliderDimensions;
    Vector2   mRenderOffset;
    Texture2D mTexture;
    int       mAtlasColumns;
    int       mAtlasRows;
    int       mFrame;

public:
    Entity(Vector2 position, Vector2 scale, const char* textureFilepath,
           int atlasColumns = 1, int atlasRows = 1);
    ~Entity();

    // An entity owns a GPU texture, so don't accidentally copy it.
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    void update(float deltaTime);
    void render() const;
    void reset(Vector2 position);

    void setPosition(Vector2 position)                   { mPosition = position; }
    void setVelocity(Vector2 velocity)                   { mVelocity = velocity; }
    void setAcceleration(Vector2 acceleration)           { mAcceleration = acceleration; }
    void setScale(Vector2 scale)                         { mScale = scale; }
    void setColliderDimensions(Vector2 dimensions)       { mColliderDimensions = dimensions; }
    void setRenderOffset(Vector2 offset)                 { mRenderOffset = offset; }
    void setFrame(int frame)                             { mFrame = frame; }

    Vector2 getPosition() const                          { return mPosition; }
    Vector2 getVelocity() const                          { return mVelocity; }
    Vector2 getAcceleration() const                      { return mAcceleration; }
    Vector2 getScale() const                             { return mScale; }
    Vector2 getColliderDimensions() const                { return mColliderDimensions; }
    Rectangle getBounds() const;
};

#endif
