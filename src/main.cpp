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

        ResetGame();

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

        if (keyboard.GetKey(olc::Key::ESCAPE).bPressed) {
            isPaused = !isPaused;
            draw.FilledRect({ 0, 0 }, ScreenSize(), overlayColor);
        }
        if (isPaused) {
            draw.FilledRoundedRect({ 94, ScreenSize().y / 2.f - 4 }, { 418, 54 }, 5, olc::Colour::VERY_DARK_GREY);
            draw.String({ 98, ScreenSize().y / 2.f }, "PAUSED\nPress ESC to Play", olc::Colour::GREY, { 3, 3 });
            return true;
        }

        bool prevGameOver = gameOver;
        if (gameTimer < 0) gameOver = true;
        if (!prevGameOver && gameOver) {
            draw.FilledRect({ 0, 0 }, ScreenSize(), overlayColor);
        }
        if (gameOver) {
            float totalScore =
                std::accumulate(coins.begin(), coins.end(), 0, [](float acc, const Coin& c) { return acc + c.value; });
            std::string s1 = "GAME OVER";
            std::string s2 = std::format("TOTAL SCORE: ${:.2f}", totalScore);
            std::string s3 = "Press SPACE to restart!";
            float p1 = ScreenSize().x / 2.0f - ((s1.size() / 2.0f) * 32);
            float p2 = ScreenSize().x / 2.0f - ((s2.size() / 2.0f) * 24);
            float p3 = ScreenSize().x / 2.0f - ((s3.size() / 2.0f) * 24);
            draw.String({ p1, ScreenSize().y / 6.f }, s1, olc::Colour::RED, { 4, 4 });
            draw.String({ p2, ScreenSize().y / 6.f + 48 }, s2, olc::Colour::WHITE, { 3, 3 });

            // TODO list of scores and how good you did!!!!!
            draw.String({ p3, ScreenSize().y / 6.f + 96 }, s3, olc::Colour::BLUE, { 3, 3 });

            if (keyboard.GetKey(olc::Key::SPACE).bReleased) {
                ResetGame();
            }
            return true;
        }

        float normRemainingGameTime = gameTimer / TOTAL_GAME_TIME;

        if (!initialCutscene) {
            bool up = keyboard.GetKey(olc::Key::UP).bHeld || keyboard.GetKey(olc::Key::W).bHeld;
            bool down = keyboard.GetKey(olc::Key::DOWN).bHeld || keyboard.GetKey(olc::Key::S).bHeld;
            bool left = keyboard.GetKey(olc::Key::LEFT).bHeld || keyboard.GetKey(olc::Key::A).bHeld;
            bool right = keyboard.GetKey(olc::Key::RIGHT).bHeld || keyboard.GetKey(olc::Key::D).bHeld;

            olc::vf2d dir;
            if (up) dir.y -= 1;
            if (down) dir.y += 1;
            if (left) dir.x -= 1;
            if (right) dir.x += 1;

            if (dir.mag2() > 0) {
                playerPos += dir.norm() * playerSpeed * fElapsedTime;
            }
        }

        if (playerPos.x < boardPosStart.x + playerSize.x) playerPos.x = boardPosStart.x + playerSize.x;
        if (playerPos.x > exchangeX - playerSize.x) playerPos.x = exchangeX - playerSize.x;
        if (playerPos.y < boardPosStart.y + playerSize.y) playerPos.y = boardPosStart.y + playerSize.y;
        if (playerPos.y > ScreenSize().y - playerSize.y) playerPos.y = ScreenSize().y - playerSize.y;

        // Player collisions with coins
        utils::geom2d::circle<float> playerCircle(playerPos,
                                                  playerSize.x); // TODO change to rect intersection with a real sprite
        for (Coin& coin : coins) {
            if (!coin.IsVisibleOnBoard()) continue;
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
            playerPos.y <= exchangeYEnd && nCoinsAcquired > 0 && !ExchangeBusy())
        {
            exchangeProcessing = true;
            exchangeProcessingTimer = TIME_EXCHANGE_ANIM;
            exCHANGE.Reset();

            exchangeRate = GetExchangeRate(nCoinsAcquired);

            for (Coin& coin : coins) {
                if (coin.isAcquired) {
                    coin.isAcquired = false;
                    coin.isExchanging = true;
                }
            }
        }
        if (exchangeProcessing) {
            exchangeProcessingTimer -= fElapsedTime;
            exCHANGE.Update(fElapsedTime);
            if (exchangeProcessingTimer < 0) {
                // After exCHANGE, Coins increase in value!
                if (!initialCutscene) {
                    for (Coin& coin : coins) {
                        if (!coin.isExchanging) continue;
                        coin.value *= exchangeRate;
                        coin.isRenderingMultiplier = true;

                        int newTileIdx;
                        do {
                            static std::uniform_int_distribution<int> randomTile(0, board.tiles.size() - 1);
                            newTileIdx = randomTile(gen);
                        } while (std::any_of(coins.begin(), coins.end(), [newTileIdx](const Coin& a) {
                            return a.tileIndex == newTileIdx;
                        }));

                        coin.tileIndex = newTileIdx;
                    }
                }
                exchangeProcessing = false;
                coinsFlying = true;
                coinsFlyingTimer = 1;
                renderExchangeRateTimer = 1;
            }
        }

        if (coinsFlying) {
            coinsFlyingTimer -= fElapsedTime;
            if (coinsFlyingTimer < 0) {
                for (Coin& coin : coins) {
                    if (coin.isExchanging) {
                        coin.isAcquired = false;
                        coin.isExchanging = false;
                    }
                }
                coinsFlying = false;
            }
        }

        if (renderExchangeRateTimer > 0)
            renderExchangeRateTimer -= fElapsedTime;
        else
            for (Coin& coin : coins)
                coin.isRenderingMultiplier = false;

        // Update board
        boardTimer -= fElapsedTime;
        if (boardTimer < 0) {
            if (boardState == NORMAL) {
                boardTimer = TIMER_DURATION_UPDATING_FADEOUT;
                boardState = UPDATING_FADEOUT;
            } else if (boardState == UPDATING_FADEOUT) {
                boardTimer = TIMER_DURATION_UPDATING_FADEIN;
                boardState = UPDATING_FADEIN;
                // Probability of safe tiles gets less and less throughout the game
                constexpr float min = 0.15;
                constexpr float max = 0.80;
                float probability = (max - min) * normRemainingGameTime + min;
                std::bernoulli_distribution randomBool(probability);
                for (int i = 0; i < board.tiles.size(); i++) {
                    board.tiles[i] = randomBool(gen);
                }
                if (initialCutscene) {
                    // Ensure first change does not insta-hurt player
                    board.tiles[board.tiles.size() / 2] = true;
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
        if (exchangeProcessing) {
            draw.Image(exCHANGE.GetFrame(), { ScreenSize().x - 80.f, boardPosStart.y }, { 2, 2 });
        } else {
            draw.Image(exCHANGE.GetStaticFrame(), { ScreenSize().x - 80.f, boardPosStart.y }, { 2, 2 });
        }

        // Light up exCHANGEr lights
        static float lightPulsingTimer = 0;
        lightPulsingTimer += fElapsedTime;
        if (lightPulsingTimer > 2) lightPulsingTimer = 0;
        olc::Pixel blendCol = lightPulsingTimer < 1 ? olc::Colour::WHITE : olc::Colour::GREY;
        draw.FilledRect(
            { 628, 182 }, { 6, 6 }, (nCoinsAcquired >= 1 ? olc::Colour::GREEN : olc::Colour::RED).blend(blendCol));
        draw.FilledRect(
            { 628, 202 }, { 6, 6 }, (nCoinsAcquired >= 2 ? olc::Colour::GREEN : olc::Colour::RED).blend(blendCol));
        draw.FilledRect(
            { 628, 224 }, { 6, 6 }, (nCoinsAcquired >= 3 ? olc::Colour::GREEN : olc::Colour::RED).blend(blendCol));
        draw.FilledRect(
            { 628, 244 }, { 6, 6 }, (nCoinsAcquired >= 4 ? olc::Colour::GREEN : olc::Colour::RED).blend(blendCol));

        // Draw separator bar
        draw.FilledRect({ 0, boardPosStart.y - 6 }, { (float)ScreenSize().x, 6 }, olc::Colour::BLACK);
        draw.FilledRect({ 1, boardPosStart.y - 5 }, { ScreenSize().x - 2.f, 4 }, olc::Colour::DARK_GREY);

        // Draw game timer bar
        draw.FilledRect({ ScreenSize().x / 2.0f, boardPosStart.y - 5 },
                        { normRemainingGameTime * (ScreenSize().x / 2.0f - 1.f), 4 },
                        olc::Colour::DARK_MAGENTA);
        draw.FilledRect({ ScreenSize().x / 2.0f, boardPosStart.y - 5 },
                        { -normRemainingGameTime * (ScreenSize().x / 2.0f - 1.f), 4 },
                        olc::Colour::DARK_MAGENTA);

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

                if (initialCutscene) {
                    color = olc::Colour::VERY_DARK_GREY;
                }

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
            if (coinsFlying && coin.isExchanging) {
                // Animate coin(s) flying back to new spot
                float size = std::sin(coinsFlyingTimer * M_PI) * 2.f + 1;
                draw.Image(coin.GetImage(*this),
                           olc::vf2d{ 570, 325 }.lerp(coin.GetPos(*this), 1.0f - coinsFlyingTimer),
                           { size, size });
            } else if (coin.IsVisibleOnBoard()) {
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

            if (coin.isRenderingMultiplier && renderExchangeRateTimer > 0) {
                olc::Pixel col = PixelLerp(olc::Colour::BLANK, olc::Colour::CYAN, renderExchangeRateTimer);
                draw.String(pos + olc::vf2d{ 40, -20 + 10 * renderExchangeRateTimer },
                            std::format("x{:.1f}", exchangeRate),
                            col);
            }

            totalScore += coin.value;
        }
        draw.String({ ScreenSize().x - 110.0f, 10 }, "TOTAL:", olc::Colour::WHITE, { 1.5, 1.5 });
        draw.String({ ScreenSize().x - 150.0f, 35 },
                    std::format("{:>7}", "$" + std::format("{:.2f}", totalScore)),
                    olc::Colour::CYAN,
                    { 2.5, 2.5 });

        if (!initialCutscene) {
            gameTimer -= fElapsedTime;
        }

        if (initialCutscene) {
            initialCutsceneTimer += fElapsedTime;
            if (initialCutsceneTimer > 4) {
                std::string s1 = "exCHANGE!";
                olc::vf2d p1 = { ScreenSize().x / 2.f - (s1.size() / 2.f) * 48, ScreenSize().y / 2.f };
                draw.FilledRoundedRect({ 94, ScreenSize().y / 2.f - 6 }, { 444, 58 }, 5, olc::Colour::VERY_DARK_GREY);
                draw.String(p1, s1, olc::Colour::WHITE, { 6, 6 });
            } else if (initialCutsceneTimer > 3) {
                std::string s1 = "SET";
                olc::vf2d p1 = { ScreenSize().x / 2.f - (s1.size() / 2.f) * 32, ScreenSize().y / 2.f };
                draw.String(p1, s1, olc::Colour::WHITE, { 4, 4 });
            } else if (initialCutsceneTimer > 2) {
                std::string s1 = "READY";
                olc::vf2d p1 = { ScreenSize().x / 2.f - (s1.size() / 2.f) * 32, ScreenSize().y / 2.f };
                draw.String(p1, s1, olc::Colour::WHITE, { 4, 4 });
            }
            if (initialCutsceneTimer > 5) {
                initialCutscene = false;
            }
        }
        return true;
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
        bool isExchanging = false; // TODO maybe make a state enum, because can't be both true at the same time
        bool IsVisibleOnBoard() const
        {
            return !isAcquired && !isExchanging;
        }

        bool isRenderingMultiplier = false;

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
    bool ExchangeBusy()
    {
        return exchangeProcessing || coinsFlying;
    }

    float exchangeRate;
    float renderExchangeRateTimer;
    float GetExchangeRate(int nCoins)
    {
        switch (nCoins) {
        case 1:
            return 1.1f;
        case 2:
            return 1.3f;
        case 3:
            return 1.6f;
        case 4:
            return 2.0f;
        default:
            return 1.0f;
        }
    }

    bool initialCutscene;
    float initialCutsceneTimer;

    bool gameOver;
    float gameTimer;
    static constexpr float TOTAL_GAME_TIME = 90.0f;

    bool isPaused;

    // Assets
    olc::Image background;
    Animator bronzeCoin;
    Animator silverCoin;
    Animator goldCoin;
    Animator gemCoin;
    Animator exCHANGE;

    // Randomness helpers
    std::mt19937 gen;

    // TODO make a PR that fixes the need to do this?
    static constexpr olc::Pixel overlayColor{ olc::Colour::BLACK.r, olc::Colour::BLACK.g, olc::Colour::BLACK.b, 150 };

    void ResetGame()
    {
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

        coins.fill(Coin());

        // TODO Start with animation that spits coins to corners
        coins[0].tileIndex = 0;
        coins[1].tileIndex = board.size.x - 1;
        coins[2].tileIndex = (board.size.y - 1) * board.size.x;
        coins[3].tileIndex = (board.size.y - 1) * board.size.x + board.size.x - 1;

        exchangeProcessing = false;
        exchangeProcessingTimer = 0;
        coinsFlying = false;
        coinsFlyingTimer = 0;

        exchangeRate = 0;
        renderExchangeRateTimer = 0;

        // Setup initial cutscene
        initialCutscene = true;
        initialCutsceneTimer = 0;
        for (Coin& coin : coins) {
            coin.isExchanging = true;
        }
        exchangeProcessing = true;
        exchangeProcessingTimer = TIME_EXCHANGE_ANIM;

        gameTimer = TOTAL_GAME_TIME;
        gameOver = false;
        isPaused = false;
    }
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
