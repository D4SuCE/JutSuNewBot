#ifndef TGBOT_STICKERSET_H
#define TGBOT_STICKERSET_H

#include "tgbot/types/Sticker.h"
#include "tgbot/types/PhotoSize.h"

#include <memory>
#include <string>
#include <vector>

namespace TgBot {

/**
 * @brief This object represents a sticker set.
 *
 * @ingroup types
 */
class StickerSet {
public:
    typedef std::shared_ptr<StickerSet> Ptr;

    /**
     * @brief Sticker set name
     */
    std::string name;

    /**
     * @brief Sticker set title
     */
    std::string title;

    /**
     * @brief True, if the sticker set contains animated stickers
     */
    bool isAnimated;

    /**
     * @brief True, if the sticker set contains masks
     */
    bool containsMasks;

    /**
     * @brief List of all set stickers
     */
    std::vector<Sticker::Ptr> stickers;

    /**
     * @brief Optional. Sticker set thumbnail in the .WEBP or .TGS format
     */
    PhotoSize::Ptr thumb;
};
}

#endif //TGBOT_STICKERSET_H
