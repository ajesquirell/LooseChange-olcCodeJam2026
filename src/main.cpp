// Define OLC_PGE3_APPLICATION to include the implementation of
// the Pixel Game Engine as part of this translation unit
#include <algorithm>
#include <string>
#define OLC_PGE3_APPLICATION
#include "olcPixelGameEngine3.h"

#define OLC_PGEX3_MINIAUDIO
#include "olcPGEX3_Miniaudio.h"

#include "utilities/olcUTIL3_Geometry2D.h"

#include <format>
#include <random>

class Animator
{
public:
    Animator() = default;
    void LoadFromFile(PixelGameEngine* pge,
                      std::string sSpriteSheetFileName,
                      int nFrames,
                      float fFramePxSize,
                      float fTimePerFrame,
                      bool playOnce = false)
    {
        LoadFromFile(pge, sSpriteSheetFileName, nFrames, fFramePxSize, fFramePxSize, fTimePerFrame, playOnce);
    }
    void LoadFromFile(PixelGameEngine* pge,
                      std::string sSpriteSheetFileName,
                      int nFrames,
                      float fFramePxSizeX,
                      float fFramePxSizeY,
                      float fTimePerFrame,
                      bool playOnce = false)
    {
        pge->CreateImageFromFile(spriteSheet, sSpriteSheetFileName);
        this->nFrames = nFrames;
        this->fFramePxSizeX = fFramePxSizeX;
        this->fFramePxSizeY = fFramePxSizeY;
        this->fTimePerFrame = fTimePerFrame;
        this->playOnce = playOnce;
    }
    void Update(float fElapsedTime)
    {
        if (playOnce && hasPlayed) return;
        fTimeCounter += fElapsedTime;
        if (fTimeCounter >= fTimePerFrame) {
            fTimeCounter -= fTimePerFrame;
            nCurrentFrame++;
            if (nCurrentFrame >= nFrames) {
                nCurrentFrame = 0;
                hasPlayed = true;
            }
        }
    }
    void Reset()
    {
        fTimeCounter = 0;
        nCurrentFrame = 0;
        hasPlayed = false;
    }
    olc::ImageRegion GetFrame()
    {
        return spriteSheet.region({ nCurrentFrame * fFramePxSizeX, 0.0f }, { fFramePxSizeX, fFramePxSizeY });
    }
    olc::ImageRegion GetStaticFrame()
    {
        // Just use first frame
        return spriteSheet.region({ 0.0f, 0.0f }, { fFramePxSizeX, fFramePxSizeY });
    }

private:
    float fTimeCounter = 0;
    int nCurrentFrame = 0;
    bool hasPlayed = false;

    olc::Image spriteSheet;
    int nFrames;
    float fFramePxSizeX;
    float fFramePxSizeY;
    float fTimePerFrame;
    bool playOnce;
};

class LooseChangeEngine : public olc::PixelGameEngine
{
public:
    LooseChangeEngine()
        : gen(std::random_device{}())
    {
        sAppName = "Loose Change - olc::CodeJam 2026";
        if (!InstallSystemExtension(&audio)) throw std::runtime_error("Failed to install olcPGEX3_Miniaudio");
    }

public:
    // Called once at the start, so create things here
    bool OnUserCreate() override
    {
        // load `assets/song1.mp3` into `song1`
        audio.CreateSoundFromFile(song1, "assets/song1.mp3");

        /**
         * this is here to demonstrate how the adventurous can
         * exploit other features of miniaudio that hasn't been
         * abstracted by the PGEX
         *
         * Here you get a pointer to a next active voice, or the
         * currently playing voice.
         */
        ma_sound_set_position(song1.GetMASound(), 0.0f, 0.0f, 0.0f);

        // load `assets/SampleA.wav` into `sample`
        audio.CreateSoundFromFile(sample, "assets/SampleA.wav");

        // create all of the waveforms at 0.1 amplitude at 440Mhz (A4)
        audio.CreateWaveform(sine, olc::ext::Miniaudio::Waveform::Type::Sine, 0.1, 440.0);
        audio.CreateWaveform(square, olc::ext::Miniaudio::Waveform::Type::Square, 0.1, 440.0);
        audio.CreateWaveform(triangle, olc::ext::Miniaudio::Waveform::Type::Triangle, 0.1, 440.0);
        audio.CreateWaveform(sawtooth, olc::ext::Miniaudio::Waveform::Type::Sawtooth, 0.1, 440.0);

        CreateImageFromFile(background, "assets/repeated-square.png");
        bronzeCoin.LoadFromFile(this, "assets/coins/bronze.png", 5, coinPxSize, 0.1);
        silverCoin.LoadFromFile(this, "assets/coins/silver.png", 5, coinPxSize, 0.1);
        goldCoin.LoadFromFile(this, "assets/coins/gold.png", 5, coinPxSize, 0.1);
        gemCoin.LoadFromFile(this, "assets/coins/gem.png", 4, coinPxSize, 0.1);
        exCHANGE.LoadFromFile(this, "assets/exCHANGEr_anim.png", 9, 40, 144, 0.1, true);
        CreateImageFromFile(exCHANGEr, "assets/exCHANGEr.png");

        boardPosStart.x = 0;
        boardPosStart.y = ScreenSize().y / 5.0f;
        boardPosEnd.x = ScreenSize().x - ScreenSize().x / 4.0f;
        boardPosEnd.y = ScreenSize().y;
        boardTimer = TIMER_DURATION_NORMAL;
        boardState = NORMAL;
        board.tiles.fill(false); // Start with all bad except for player tile - TODO should we do this??
        board.tiles[board.tiles.size() / 2] = true;

        olc::vf2d boardPxSize = boardPosEnd - boardPosStart;

        // Player start on middle tile, which is always safe to start
        playerPos = { boardPosStart.x + boardPxSize.x / 2.f, boardPosStart.y + boardPxSize.y / 2.f };
        playerSize = { 5, 5 };

        tileSize = { boardPxSize.x / board.size.x, boardPxSize.y / board.size.y };

        // TODO Start with animation that spits coins to corners
        coins[0].tileIndex = 0;
        coins[1].tileIndex = board.size.x - 1;
        coins[2].tileIndex = (board.size.y - 1) * board.size.x;
        coins[3].tileIndex = (board.size.y - 1) * board.size.x + board.size.x - 1;

        exchangeProcessing = false;
        exchangeProcessingTimer = 0;
        coinsFlying = false;
        coinsFlyingTimer = 0;

        return true;
    }

    // Called every frame, so update things here
    bool OnUserUpdate(float fElapsedTime) override
    {
        // // toggle background playback
        // if (keyboard.GetKey(olc::Key::K1).bPressed) {
        //   backgroundPlay = !backgroundPlay;
        //   if (backgroundPlay)
        //     audio.EnableBackgroundPlayback();
        //   else
        //     audio.DisableBackgroundPlayback();
        // }

        // panning
        // if (keyboard.GetKey(olc::Key::MINUS).bHeld)
        //   pan -= 1.0f * fElapsedTime;
        //
        // if (keyboard.GetKey(olc::Key::EQUALS).bHeld)
        //   pan += 1.0f * fElapsedTime;
        //
        // // pitch
        // if (keyboard.GetKey(olc::Key::OEM_4).bHeld)
        //   pitch -= 1.0f * fElapsedTime;
        //
        // if (keyboard.GetKey(olc::Key::OEM_6).bHeld)
        //   pitch += 1.0f * fElapsedTime;
        //
        // // volume
        // if (keyboard.GetKey(olc::Key::DOWN).bHeld)
        //   volume -= 1.0f * fElapsedTime;
        //
        // if (keyboard.GetKey(olc::Key::UP).bHeld)
        //   volume += 1.0f * fElapsedTime;
        //
        // // distance
        // if (keyboard.GetKey(olc::Key::LEFT).bHeld)
        //   distance -= 10.0f * fElapsedTime;
        //
        // if (keyboard.GetKey(olc::Key::RIGHT).bHeld)
        //   distance += 10.0f * fElapsedTime;

        // panning
        // pan = std::clamp(pan, -1.0f, 1.0f);
        // song1.SetPan(pan);
        //
        // // pitch
        // pitch = std::clamp(pitch, 0.0f, 2.0f);
        // song1.SetPitch(pitch);
        //
        // // volume
        // volume = std::clamp(volume, 0.0f, 1.0f);
        // song1.SetVolume(volume);

        // this is here to demosntrate how the adventurous can exploit other
        // features of miniaudio that haven't been abstracted by the PGEX.
        // distance = std::clamp(distance, 0.0f, 100.0f);
        // ma_engine_listener_set_position(&audio.GetEngine(), 0, 0.0f, distance,
        //                                 0.0f);

        olc::vf2d dir;
        if (keyboard.GetKey(olc::Key::UP).bHeld) dir.y -= 1;
        if (keyboard.GetKey(olc::Key::DOWN).bHeld) dir.y += 1;
        if (keyboard.GetKey(olc::Key::LEFT).bHeld) dir.x -= 1;
        if (keyboard.GetKey(olc::Key::RIGHT).bHeld) dir.x += 1;

        if (dir.mag2() > 0) {
            playerPos += dir.norm() * playerSpeed * fElapsedTime;
        }

        // TODO fix speed when both directions pressed

        if (playerPos.x < boardPosStart.x + playerSize.x) playerPos.x = boardPosStart.x + playerSize.x;
        if (playerPos.x > exchangeX - playerSize.x) playerPos.x = exchangeX - playerSize.x;
        if (playerPos.y < boardPosStart.y + playerSize.y) playerPos.y = boardPosStart.y + playerSize.y;
        if (playerPos.y > ScreenSize().y - playerSize.y) playerPos.y = ScreenSize().y - playerSize.y;

        // Player collisions with coins
        utils::geom2d::circle<float> playerCircle(playerPos,
                                                  playerSize.x); // TODO change to rect intersection with a real sprite
        for (Coin& coin : coins) {
            if (coin.isAcquired) continue;
            olc::vf2d coinPos = coin.GetPos(*this);
            utils::geom2d::rect<float> coinRect({ coinPos.x, coinPos.y }, { coinPxSize, coinPxSize });

            if (utils::geom2d::overlaps(playerCircle, coinRect)) {
                coin.isAcquired = true;
            }
        }

        int playerBoardX = (playerPos.x - boardPosStart.x) / tileSize.x;
        int playerBoardY = (playerPos.y - boardPosStart.y) / tileSize.y;

        int playerTileIdx = -1;
        if (playerBoardX >= 0 && playerBoardX < board.size.x && playerBoardY >= 0 && playerBoardY < board.size.y) {
            // Player inside board
            playerTileIdx = playerBoardY * board.size.x + playerBoardX;
        }
        bool playerInTheRed = playerTileIdx >= 0 && !board.tiles[playerTileIdx] && boardState == NORMAL;

        int nCoinsAcquired = std::accumulate(coins.begin(), coins.end(), 0, [](int acc, const Coin& coin) {
            return acc + (int)coin.isAcquired;
        });
        float exchangeRate = 1.f;
        switch (nCoinsAcquired) {
        case 1:
            exchangeRate = 1.10f;
            break;
        case 2:
            exchangeRate = 1.25f;
            break;
        case 3:
            exchangeRate = 1.5f;
            break;
        case 4:
            exchangeRate = 2.0f;
            break;
        }

        // Update Coins
        for (Coin& coin : coins) {
            float inflationRate = 0;
            if (coin.isAcquired) inflationRate = baseInflationRate;
            if (playerInTheRed) inflationRate = megaInflationRate; // ...MASSIVE INFLATION to all coins!!!
            if (inflationRate == 0) continue;

            // Inflation is happening to decrease value!
            coin.value -= (coin.value * inflationRate * fElapsedTime);
            if (coin.value < 0) coin.value = 0;
        }

        // Handle player at the exCHANGE
        if (playerPos.x >= exchangeX - playerSize.x - 2 && playerPos.y >= exchangeYStart &&
            playerPos.y <= exchangeYEnd && nCoinsAcquired > 0 && !exchangeProcessing && !coinsFlying)
        {
            exchangeProcessing = true;
            exchangeProcessingTimer = TIME_EXCHANGE_ANIM;
            exCHANGE.Reset();
        }
        if (exchangeProcessing) {
            exchangeProcessingTimer -= fElapsedTime;
            exCHANGE.Update(fElapsedTime);
            if (exchangeProcessingTimer < 0) {
                // After exCHANGE, Coins increase in value!
                for (Coin& coin : coins) {
                    if (!coin.isAcquired) continue;
                    static std::uniform_int_distribution<int> randomTile(0, board.tiles.size() - 1);

                    // coin.value += coin.value * exchangeRate;
                    coin.value *= exchangeRate;

                    int newTileIdx;
                    do {
                        newTileIdx = randomTile(gen);
                    } while (std::any_of(coins.begin(), coins.end(), [newTileIdx](const Coin& a) {
                        return a.tileIndex == newTileIdx;
                    }));

                    coin.tileIndex = newTileIdx;
                }
                exchangeProcessing = false;
                coinsFlying = true;
                coinsFlyingTimer = 1;
            }
        }

        if (coinsFlying) {
            coinsFlyingTimer -= fElapsedTime;
            if (coinsFlyingTimer < 0) {
                for (Coin& coin : coins) {
                    if (!coin.isAcquired) continue;
                    coin.isAcquired = false;
                }
                coinsFlying = false;
            }
        }

        // Update board
        boardTimer -= fElapsedTime;
        if (boardTimer < 0) {
            if (boardState == NORMAL) {
                boardTimer = TIMER_DURATION_UPDATING_FADEOUT;
                boardState = UPDATING_FADEOUT;
            } else if (boardState == UPDATING_FADEOUT) {
                boardTimer = TIMER_DURATION_UPDATING_FADEIN;
                boardState = UPDATING_FADEIN;
                for (int i = 0; i < board.tiles.size(); i++) {
                    // TODO need to be more aggressive as time goes on, or as values get higher
                    static std::uniform_int_distribution<int> randomBool(0, 1);
                    board.tiles[i] = randomBool(gen);
                }
            } else {
                boardTimer = TIMER_DURATION_NORMAL;
                boardState = NORMAL;
            }
        }

        /* === DRAWING === */

        draw.Clear(olc::Colour::VERY_DARK_BLUE);
        draw.ImageRect(background, { 0, boardPosStart.y }, ScreenSize());

        // Draw exCHANGEr
        if (nCoinsAcquired > 0) {
            static float pulsingTimer = 0;
            pulsingTimer += fElapsedTime;
            float vignetteSize = 10.0f + std::cos(pulsingTimer * 2 * M_PI) * -5;
            olc::Pixel col = nCoinsAcquired < 4 ? olc::Colour::YELLOW : olc::Colour::GREEN;
            olc::Pixel blank = olc::Colour::BLANK;
            draw.FilledRect({ exchangeX - vignetteSize, 168 }, { vignetteSize, 96 }, blank, col, blank, col);
        }
        // draw.Image(exCHANGEr, { ScreenSize().x - 80.f, boardPosStart.y }, { 2, 2 });
        if (exchangeProcessing) {
            draw.Image(exCHANGE.GetFrame(), { ScreenSize().x - 80.f, boardPosStart.y }, { 2, 2 });
        } else {
            draw.Image(exCHANGE.GetStaticFrame(), { ScreenSize().x - 80.f, boardPosStart.y }, { 2, 2 });
        }

        // Light up exCHANGEr lights
        draw.FilledRect({ 628, 182 }, { 6, 6 }, nCoinsAcquired >= 1 ? olc::Colour::GREEN : olc::Colour::RED);
        draw.FilledRect({ 628, 202 }, { 6, 6 }, nCoinsAcquired >= 2 ? olc::Colour::GREEN : olc::Colour::RED);
        draw.FilledRect({ 628, 224 }, { 6, 6 }, nCoinsAcquired >= 3 ? olc::Colour::GREEN : olc::Colour::RED);
        draw.FilledRect({ 628, 244 }, { 6, 6 }, nCoinsAcquired >= 4 ? olc::Colour::GREEN : olc::Colour::RED);

        // Draw separator bar
        draw.FilledRect({ 0, boardPosStart.y - 6 }, { (float)ScreenSize().x, 6 }, olc::Colour::BLACK);
        draw.FilledRect({ 1, boardPosStart.y - 5 }, { ScreenSize().x - 1.f, 4 }, olc::Colour::DARK_GREY);

        // TODO I actually don't think I like the bar timer
        // float normBarTime = 0;
        // if (boardState == NORMAL) {
        //     float barDuration = TIMER_DURATION_NORMAL;
        //     normBarTime = boardTimer / barDuration;
        // } else {
        //     float renderTimer = boardTimer;
        //     if (boardState == UPDATING_FADEOUT) {
        //         renderTimer += TIMER_DURATION_UPDATING_FADEIN;
        //     }
        //     float barDuration = TIMER_DURATION_UPDATING_FADEOUT + TIMER_DURATION_UPDATING_FADEIN;
        //     normBarTime = 1.0f - renderTimer / barDuration;
        // }
        // draw.FilledRect(
        //     { 1, boardPosStart.y - 5 }, { normBarTime * (ScreenSize().x - 1.f), 4 }, olc::Colour::DARK_MAGENTA);

        // Update coin animations
        bronzeCoin.Update(fElapsedTime);
        silverCoin.Update(fElapsedTime);
        goldCoin.Update(fElapsedTime);
        gemCoin.Update(fElapsedTime);

        // Draw Board
        for (int x = 0; x < board.size.x; x++) {
            for (int y = 0; y < board.size.y; y++) {
                olc::vf2d pos = GetTilePos(x, y);
                std::size_t tile_index = y * board.size.x + x;
                olc::Pixel color = board.tiles[tile_index] ? olc::Colour::BLUE : olc::Colour::RED;

                switch (boardState) {
                case UPDATING_FADEOUT: {
                    float t = boardTimer / TIMER_DURATION_UPDATING_FADEOUT; // Counts up
                    color.a = t * 255;
                } break;

                case UPDATING_FADEIN: {
                    float t = 1.0f - boardTimer / TIMER_DURATION_UPDATING_FADEIN; // Counts down
                    color.a = t * 255;

                    // Epilepsy mode...
                    // if (std::fmod(t, 0.05) < 0.025) color.a = 0;
                } break;

                default:
                    break;
                }

                if (playerTileIdx == tile_index) {
                    color = color.blend(olc::Colour::GREY);
                }

                draw.FilledRect(pos, tileSize, color);
                draw.Rect(pos, tileSize, olc::Colour::BLACK);
            }
        }

        // Draw Coins
        for (const Coin& coin : coins) {
            if (coinsFlying && coin.isAcquired) {
                // Animate coin(s) flying back to new spot
                float size = std::sin(coinsFlyingTimer * M_PI) * 2.f + 1;
                draw.Image(coin.GetImage(*this),
                           olc::vf2d{ 570, 325 }.lerp(coin.GetPos(*this), 1.0f - coinsFlyingTimer),
                           { size, size });
            } else if (!coin.isAcquired) {
                draw.Image(coin.GetImage(*this), coin.GetPos(*this));
            }
        }

        // Draw Player
        draw.FilledCircle(playerPos, 5, olc::Colour::GREEN);

        // Draw pulsing box shadow around screen
        static float pulsingTimer = 0;
        if (playerInTheRed) {
            float width = ScreenSize().x;
            float height = ScreenSize().y;
            olc::Pixel col = olc::Colour::RED;
            olc::Pixel blank = olc::Colour::BLANK;

            pulsingTimer += fElapsedTime;
            float vignetteSize = 30.0f + std::cos(pulsingTimer * 2 * M_PI) * -10;

            draw.FilledRect({ 0, 0 }, { width, vignetteSize }, col, col, blank, blank);
            draw.FilledRect({ 0, height - vignetteSize }, { width, vignetteSize }, blank, blank, col, col);
            draw.FilledRect({ 0, 0 }, { vignetteSize, height }, col, blank, col, blank);
            draw.FilledRect({ width - vignetteSize, 0 }, { vignetteSize, height }, blank, col, blank, col);
        } else {
            pulsingTimer = 0;
        }

        // Draw top info

        if (playerInTheRed) {
            draw.String({ 25, 25 },
                        std::format("MEGA\nInflation!", megaInflationRate * 100),
                        olc::Colour::DARK_RED,
                        { 1.8, 1.8 });
        } else {
            draw.String({ 5, 20 }, std::format("Inflation: {:.0f}% / sec", baseInflationRate * 100));
            draw.String({ 5, 40 }, std::format("exCHANGE rate: {:.2f}%", exchangeRate));
        }

        float totalScore = 0;
        for (int i = 0; i < coins.size(); i++) {
            const Coin& coin = coins[i];
            constexpr float scale = 1.5f;
            constexpr float distBtwCoins = 50.0f;
            olc::vf2d pos{ ScreenSize().x / 2.0f - (coinPxSize * 2) - (distBtwCoins * 1.5f),
                           boardPosStart.y / 2.0f - (coinPxSize * scale) / 2.0f - 10.0f };
            pos.x += (coinPxSize + distBtwCoins) * i;
            draw.Image(coin.GetStaticImage(*this), pos, { scale, scale });

            pos.x -= 15;
            pos.y += coinPxSize + 15;
            draw.String(pos, std::format("{:^7}", "$" + std::format("{:.2f}", coin.value)));

            totalScore += coin.value;
        }
        draw.String({ ScreenSize().x - 110.0f, 10 }, "TOTAL:", olc::Colour::WHITE, { 1.5, 1.5 });
        draw.String({ ScreenSize().x - 150.0f, 35 },
                    std::format("{:>7}", "$" + std::format("{:.2f}", totalScore)),
                    olc::Colour::CYAN,
                    { 2.5, 2.5 });

        // DEBUG INFO
        // draw.String({ 5, 5 }, std::to_string(playerTileIdx));
        // draw.String({ 5, 5 }, std::format("{:.2f}{:.2f}", playerPos.x, playerPos.y));

#if OLC_HOST == OLC_HOST_EMSCRIPTEN
        return true;
#else
        return !keyboard.GetKey(olc::Key::ESCAPE).bPressed;
#endif
    }

    // put this here to have access to audio!
    olc::ext::Miniaudio::AudioEngine audio;

private:
    // sounds
    olc::ext::Miniaudio::Sound song1;
    olc::ext::Miniaudio::Sound sample;

    olc::ext::Miniaudio::Waveform sine;
    olc::ext::Miniaudio::Waveform square;
    olc::ext::Miniaudio::Waveform triangle;
    olc::ext::Miniaudio::Waveform sawtooth;

    // For demonstration controls, with sensible default values
    float pan = 0.0f;
    float pitch = 1.0f;
    float volume = 1.0f;
    float distance = 0.0f;

    olc::vf2d playerPos;
    olc::vf2d playerSize;
    const float playerSpeed = 150.0f;

    struct Board
    {
        static constexpr olc::vu2d size{ 5, 5 };
        std::array<bool, size.x * size.y> tiles;
    };

    Board board;
    olc::vf2d boardPosStart;
    olc::vf2d boardPosEnd;
    float boardTimer;
    enum { NORMAL, UPDATING_FADEOUT, UPDATING_FADEIN } boardState;

    static constexpr float TIMER_DURATION_NORMAL = 4;
    static constexpr float TIMER_DURATION_UPDATING_FADEOUT = 0.5f;
    static constexpr float TIMER_DURATION_UPDATING_FADEIN = 1.5f;

    olc::vf2d tileSize;

    olc::vf2d GetTilePos(int xIndex, int yIndex)
    {
        // - 1 because it will overdraw off the screen
        // TODO maybe give it a small buffer to look better, then
        // we don't need this?
        return { boardPosStart.x + tileSize.x * xIndex, boardPosStart.y + tileSize.y * yIndex - 1 };
    }

    struct Coin
    {
        float value =
            1.0f; // TODO Allow the numbers to be big enough to be fun. Maybe start at silver so bronze is like a penalty. Maybe you hit bronze quick enough after picking up to show user how it works
        uint32_t tileIndex = 0;
        bool isAcquired = false;

        // TODO consider having coin positions in float space - might make it more interesting if they are between tiles
        olc::vf2d GetPos(LooseChangeEngine& engine) const
        {
            int xIndex = tileIndex % engine.board.size.x;
            int yIndex = tileIndex / engine.board.size.x;
            olc::vf2d tilePos = engine.GetTilePos(xIndex, yIndex);
            return tilePos + engine.tileSize / 2.f - engine.coinPxSize / 2.f;
        }

        olc::ImageRegion GetImage(LooseChangeEngine& engine) const
        {
            return get_animator(engine).GetFrame();
        }

        olc::ImageRegion GetStaticImage(LooseChangeEngine& engine) const
        {
            return get_animator(engine).GetStaticFrame();
        }

    private:
        Animator& get_animator(LooseChangeEngine& engine) const
        {
            if (value > 10.0f) {
                return engine.gemCoin;
            }
            if (value > 5.0f) {
                return engine.goldCoin;
            }
            if (value > 2.0f) {
                return engine.silverCoin;
            }
            return engine.bronzeCoin;
        }
    };
    std::array<Coin, 4> coins;
    static constexpr int coinPxSize = 16;

    // TODO ORRRRRR stepping on red increases your CURRENT inflation rate for the rest of the game!!!! but then user can never recover....
    static constexpr float baseInflationRate = 0.03f; // Percent per second
    static constexpr float megaInflationRate = baseInflationRate * 4;

    // Static increments where inflation rate increases, per coin (seconds)
    static constexpr float HELD_TIME_PENALTY = 5.0f;
    static constexpr int PENALTY_INCREMENT_LIMIT = 4;

    static constexpr float exchangeX = 560.f;
    static constexpr float exchangeYStart = 168.f;
    static constexpr float exchangeYEnd = 264.f;

    bool exchangeProcessing;
    float exchangeProcessingTimer;

    static constexpr float TIME_EXCHANGE_ANIM = 0.9f;

    bool coinsFlying;
    float coinsFlyingTimer;

    // Assets
    olc::Image background;
    Animator bronzeCoin;
    Animator silverCoin;
    Animator goldCoin;
    Animator gemCoin;
    olc::Image exCHANGEr;
    Animator exCHANGE;

    // Randomness helpers
    std::mt19937 gen;
};

// Main entry point for the application
int main()
{
    // Construct demo application
    LooseChangeEngine game;

    PGEConfig config;
    config.bVSync = false;
    config.vPixelSize = { 2, 2 };
    config.vScreenSize = { 640, 360 };

    if (game.Construct(config)) {
        // Start the application
        game.Start();
    }

    return 0;
}
