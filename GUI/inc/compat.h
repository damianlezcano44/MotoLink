/*
This file is part of QSTLink2.

    QSTLink2 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QSTLink2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QSTLink2.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef COMPAT_H
#define COMPAT_H

#if defined(_WIN32) || defined(_WIN64)
#define WINDOWS 1
#endif

#define PrintError() qCritical("In %s, at %s:%d", Q_FUNC_INFO, __FILE__, __LINE__)
#define PrintFuncName() qDebug() << "***[" << Q_FUNC_INFO << "]***"

#endif // COMPAT_H
