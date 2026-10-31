#include "Entity.h"

Entity::Entity(Vector2 position, Vector2 scale, const char* textureFilepath,
               int atlasColumns, int atlasRows) :
    mPosition {position},
    mVelocity {0.0f, 0.0f},
    mAcceleration {0.0f, 0.0f},
    mScale {scale},
    mColliderDimensions {scale},
    mRenderOffset {0.0f, 0.0f},
    mTexture {LoadTexture(textureFilepath)},
    mAtlasColumns {atlasColumns},
    mAtlasRows {atlasRows},
    mFrame {0}
{
    SetTextureFilter(mTexture, TEXTURE_FILTER_POINT);
}

Entity::~Entity()
{
    if (mTexture.id != 0) UnloadTexture(mTexture);
}

void Entity::reset(Vector2 position)
{
    mPosition = position;
    mVelocity = {0.0f, 0.0f};
    mAcceleration = {0.0f, 0.0f};
    mFrame = 0;
}

void Entity::update(float deltaTime)
{
    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y += mAcceleration.y * deltaTime;

    if (mVelocity.x > 65.0f)  mVelocity.x = 65.0f;
    if (mVelocity.x < -65.0f) mVelocity.x = -65.0f;
    if (mVelocity.y > 75.0f)  mVelocity.y = 75.0f;
    if (mVelocity.y < -85.0f) mVelocity.y = -85.0f;

    mPosition.x += mVelocity.x * deltaTime;
    mPosition.y += mVelocity.y * deltaTime;
}

Rectangle Entity::getBounds() const
{
    return {mPosition.x, mPosition.y,
            mColliderDimensions.x, mColliderDimensions.y};
}

void Entity::render() const
{
    if (mTexture.id == 0) return;

    const int columns = mAtlasColumns > 0 ? mAtlasColumns : 1;
    const int rows = mAtlasRows > 0 ? mAtlasRows : 1;
    const float frameWidth = static_cast<float>(mTexture.width) / columns;
    const float frameHeight = static_cast<float>(mTexture.height) / rows;
    const int col = mFrame % columns;
    const int row = mFrame / columns;

    Rectangle source = {
        col * frameWidth, row * frameHeight, frameWidth, frameHeight
    };
    Rectangle destination = {
        mPosition.x + mRenderOffset.x, mPosition.y + mRenderOffset.y,
        mScale.x, mScale.y
    };
    DrawTexturePro(mTexture, source, destination, {0.0f, 0.0f}, 0.0f, WHITE);
}
