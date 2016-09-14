/* InlineDateAlgorithms
 * Copyright (C) 2012 by William Greiman
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InlineDateAlgorithms.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef InlineDateAlgorithms_h
#define InlineDateAlgorithms_h
//------------------------------------------------------------------------------
#ifndef EPOCH_YEAR
/** default Epoch is January 1 00:00:00 of EPOCH_YEAR */
#define EPOCH_YEAR 1970
#endif  // EPOCH_YEAR
//------------------------------------------------------------------------------
/** is leap year
 * \param[in] y year, 1900 < y < 2100
 * \return one if leap year else zero
 */
inline bool leap(uint16_t y) {return (y & 3) == 0;}
//------------------------------------------------------------------------------
/** Day of year to month
 * \param[in] y year 1900 < y < 2100
 * \param[in] yday day of year 0 <= yday <= 365
 * \return month [1,12]
 */
inline uint8_t dayOfYearToMonth(uint16_t y, uint16_t yday) {
  return yday < 31 ? 1 : (456 + 5 * (yday - 58 - leap(y))) / 153;
}
//------------------------------------------------------------------------------
/** Number of days in the year before the current month
 * \param[in] y year 1900 < y < 2100
 * \param[in] m month 0 < m < 13
 * \return days in year before current month [0,335]
 */
inline uint16_t daysBeforeMonth(uint16_t y, uint8_t m) {
  return m < 3 ? 31 *(m - 1) : leap(y) + (153 * m - 2) / 5 - 32;
}
//------------------------------------------------------------------------------
/** Count of days since epoch in previous years.
 * \param[in] y year (EPOCH_YEAR <= y <= MAX_YEAR)
 * \return count of days in previous years
 */
inline uint16_t daysBeforeYear(uint16_t y) {
  return 365 * (y - EPOCH_YEAR) + (y - 1) / 4 - (EPOCH_YEAR - 1) / 4;
}
//------------------------------------------------------------------------------
/** Number of days in month
 * \param[in] y year 1900 < y < 2100
 * \param[in] m month 0 < m < 13
 * \return Count of days in month [1, 31]
 */
inline uint8_t daysInMonth(uint16_t y, uint8_t m) {
  return m == 2 ? 28 + leap(y) : m < 8 ? 30 + (m & 1) : 31 - (m & 1);
}
//------------------------------------------------------------------------------
/** Count of days since Epoch.
 * 1900 < EPOCH_YEAR, MAX_YEAR < 2100, (MAX_YEAR - EPOCH_YEAR) < 178.
 * \param[in] y year (EPOCH_YEAR <= y <= MAX_YEAR)
 * \param[in] m month 1 <= m <= 12
 * \param[in] d day 1 <= d <= 31
 * \return Count of days since epoch
 *
 * Derived from Zeller's congruence
 */
inline uint16_t daysSinceEpoch(uint16_t y, uint8_t m, uint8_t d) {
  if (m < 3) {
    m += 12;
    y--;
  }
  return 365 * (y + 1 - EPOCH_YEAR)  + y / 4 - (EPOCH_YEAR - 1) / 4
    + (153 * m - 2) / 5 + d - 398;
}
//------------------------------------------------------------------------------
/** epoch day to day of week (Sunday == 0)
 * \param[in] eday count of days since epoch.
 * \return day of week (Sunday == 0)
 **/
inline uint8_t epochDayToDayOfWeek(uint16_t eday) {
  return (eday + EPOCH_YEAR - 1 + (EPOCH_YEAR - 1) / 4) % 7;
}
//------------------------------------------------------------------------------
/** Day of epoch to year
 * \param[in] eday count of days since epoch
 * \return year for count of days since epoch
 */
inline uint16_t epochDayToYear(uint16_t eday) {
  return EPOCH_YEAR
    + (eday - (eday + 365 * (1 + (EPOCH_YEAR - 1) % 4)) / 1461) / 365;
}
#endif  // InlineDateAlgorithms_h