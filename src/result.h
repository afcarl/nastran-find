/* - NASTRANFIND - Copyright (C) 2016-2017 Sebastien Vavassori
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <vector>
#include <map>

typedef std::vector<std::string> stringlist;
class Result
{
public:
    explicit Result() : occurrenceCount(0) {}

    stringlist::size_type occurrenceCount;
    stringlist occurrences;
};

typedef std::map<std::string, Result> ResultMap;


#endif // RESULT_H
