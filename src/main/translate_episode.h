/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef TRANSLATEEPISODE_H
#define TRANSLATEEPISODE_H

#include <unordered_map>
#include <string>


class TranslateEpisode
{
    std::unordered_map<int, std::string> m_scriptLines;
    std::unordered_map<std::string, std::string> m_scriptTrId;

public:
    TranslateEpisode();

    /*!
     * \brief Translate the level file data by key
     * \param lvl Level data
     * \param key Translation key
     */
    void loadLevelTranslation(const std::string &key);

    /*!
     * \brief Translate the world map file data by key
     * \param wld World map data
     * \param key Translation key
     */
    void loadWorldTranslation(const std::string &key);

    /*!
     * \brief Load the script translation data by key
     * \param key Translation key
     */
    void loadLunaScript(const std::string &key);

    /*!
     * \brief Attempt to find the translated episode title
     * \param episodePath Path to the episode directory to prove
     * \return true if translation has been found
     */
    bool tryTranslateTitle(const std::string &episodePath, const std::string &worldFile, std::string &output);

    /*!
     * \brief Translate script line by number
     * \param data Line data to translate
     * \param line line number where the line appears
     */
    void trScriptLine(std::string &data, int line);
};

#endif // TRANSLATEEPISODE_H
