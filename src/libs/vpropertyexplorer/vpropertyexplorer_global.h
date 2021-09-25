/************************************************************************
 **
 **  @file   vpropertyexplorer_global.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 8, 2014
 **
 **  @brief
 **  @copyright
 **  All rights reserved. This program and the accompanying materials
 **  are made available under the terms of the GNU Lesser General Public License
 **  (LGPL) version 2.1 which accompanies this distribution, and is available at
 **  http://www.gnu.org/licenses/lgpl-2.1.html
 **
 **  This library is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 **  Lesser General Public License for more details.
 **
 *************************************************************************/

#ifndef VPROPERTYEXPLORER_GLOBAL_H
#define VPROPERTYEXPLORER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(VPROPERTYEXPLORER_LIBRARY)
#  define VPROPERTYEXPLORERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define VPROPERTYEXPLORERSHARED_EXPORT Q_DECL_IMPORT
#endif

#ifndef VPE_ATTRIBUTE_UNUSED
# if defined(Q_CC_GNU) || defined(Q_CC_CLANG)
#  define VPE_ATTRIBUTE_UNUSED [[gnu::unused]]
#elif defined(Q_CC_MSVC)
#  define VPE_ATTRIBUTE_UNUSED __declspec(unused)
# else
#  define VPE_ATTRIBUTE_UNUSED
# endif
#endif

// Example of use
//class Base
//{
//    virtual ~Base()
//    {
//    }

//    virtual int a(float f)
//    {
//    }
//    virtual void b(double)
//    {
//    }
//};

//VPE_MARK_NONFINAL_CLASS(Base)
//VPE_ATTRIBUTE_UNUSED(Base, int, a(float))
//VPE_ATTRIBUTE_UNUSED(Base, void, b(double)

#ifndef VPE_MARK_NONFINAL_CLASS
#define VPE_MARK_NONFINAL_CLASS(base)                        \
    namespace vpe_void_namespace_for_class_##base            \
    {                                                        \
        struct VPE_ATTRIBUTE_UNUSED temp_marker final : base \
        {                                                    \
        };                                                   \
    }
#endif

#ifndef VPE_MARK_NONFINAL_METHOD
#define VPE_MARK_NONFINAL_METHOD(base, return_type, method)            \
    namespace vpe_void_namespace_for_class_##base##_methos_##method    \
    {                                                                  \
        struct VPE_ATTRIBUTE_UNUSED temp_marker final : base           \
        {                                                              \
            inline return_type VPE_ATTRIBUTE_UNUSED method override {} \
        };                                                             \
    }
#endif

#endif // VPROPERTYEXPLORER_GLOBAL_H
