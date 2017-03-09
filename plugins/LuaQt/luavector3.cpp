#include "luavector3.h"

#include <fugio/node_interface.h>
#include <fugio/node_control_interface.h>
#include <fugio/pin_interface.h>
#include <fugio/pin_control_interface.h>
#include <fugio/context_interface.h>
#include <fugio/lua/lua_interface.h>
#include <fugio/core/variant_interface.h>

#include "luaqtplugin.h"

#include "luapointf.h"
#include "luarectf.h"

const char *LuaVector3D::UserData::TypeName = "qt.vector3d";

#if defined( LUA_SUPPORTED )

const luaL_Reg LuaVector3D::mLuaInstance[] =
{
	{ "new",				LuaVector3D::luaNew },
	{ 0, 0 }
};

const luaL_Reg LuaVector3D::mLuaMethods[] =
{
//	{ "__add",				LuaVector3D::luaAdd },
//	{ "__div",				LuaVector3D::luaDiv },
//	{ "__eq",				LuaVector3D::luaEq },
//	{ "__mul",				LuaVector3D::luaMul },
	{ "__sub",				LuaVector3D::luaSub },
	{ "crossProduct",		LuaVector3D::luaCrossProduct },
	{ "dotProduct",			LuaVector3D::luaDotProduct },
	{ "length",				LuaVector3D::luaLength },
	{ "normalize",			LuaVector3D::luaNormalize },
	{ "normalized",			LuaVector3D::luaNormalized },
	{ "toArray",			LuaVector3D::luaToArray },
	{ "x",					LuaVector3D::luaX },
	{ "y",					LuaVector3D::luaY },
	{ "z",					LuaVector3D::luaZ },
	{ 0, 0 }
};

int LuaVector3D::luaOpen (lua_State *L )
{
	if( luaL_newmetatable( L, UserData::TypeName ) == 1 )
	{
		lua_pushvalue( L, -1 );
		lua_setfield( L, -2, "__index" );

		luaL_setfuncs( L, mLuaMethods, 0 );

		luaL_newlib( L, mLuaInstance );
	}

	return( 1 );
}

int LuaVector3D::luaNew( lua_State *L )
{
	QVector3D		V;

	if( lua_gettop( L ) == 1 )
	{
		if( lua_type( L, 1 ) == LUA_TTABLE )
		{
			for( int i = 1 ; i <= 3 ; i++ )
			{
				lua_rawgeti( L, 1, i );

				if( lua_isnil( L, -1 ) )
				{
					lua_pop( L, 1 );

					break;
				}

				if( i == 1 ) V.setX( lua_tonumber( L, -1 ) );
				if( i == 2 ) V.setY( lua_tonumber( L, -1 ) );
				if( i == 3 ) V.setZ( lua_tonumber( L, -1 ) );

				lua_pop( L, 1 );
			}
		}
	}
	else if( lua_gettop( L ) == 3 )
	{
		float		x = luaL_checknumber( L, 1 );
		float		y = luaL_checknumber( L, 2 );
		float		z = luaL_checknumber( L, 3 );

		V = QVector3D( x, y, z );
	}

	pushvector3d( L, V );

	return( 1 );
}

int LuaVector3D::luaPinGet( const QUuid &pPinLocalId, lua_State *L )
{
	fugio::LuaInterface						*Lua  = LuaQtPlugin::lua();
	NodeInterface							*Node = Lua->node( L );
	QSharedPointer<fugio::PinInterface>		 Pin = Node->findPinByLocalId( pPinLocalId );
	QSharedPointer<fugio::PinInterface>		 PinSrc;

	if( !Pin )
	{
		return( luaL_error( L, "No source pin" ) );
	}

	if( Pin->direction() == PIN_OUTPUT )
	{
		PinSrc = Pin;
	}
	else
	{
		PinSrc = Pin->connectedPin();
	}

	if( !PinSrc || !PinSrc->hasControl() )
	{
		return( luaL_error( L, "No vector3 pin" ) );
	}

	fugio::VariantInterface			*SrcVar = qobject_cast<fugio::VariantInterface *>( PinSrc->control()->qobject() );

	if( !SrcVar )
	{
		return( luaL_error( L, "Can't access vector3" ) );
	}

	return( pushvector3d( L, SrcVar->variant().value<QVector3D>() ) );
}

int LuaVector3D::luaCrossProduct( lua_State *L )
{
	QVector3D		v1 = checkvector3d( L, 1 );
	QVector3D		v2 = checkvector3d( L, 2 );

	pushvector3d( L, QVector3D::crossProduct( v1, v2 ) );

	return( 1 );
}

int LuaVector3D::luaDotProduct( lua_State *L )
{
	QVector3D		v1 = checkvector3d( L, 1 );
	QVector3D		v2 = checkvector3d( L, 2 );

	lua_pushnumber( L, QVector3D::dotProduct( v1, v2 ) );

	return( 1 );
}

int LuaVector3D::luaSub( lua_State *L )
{
	UserData			*V1 = checkvector3duserdata( L );

	luaL_checkany( L, 2 );

	if( isVector3D( L, 2 ) )
	{
		UserData			*V2 = checkvector3duserdata( L, 2 );
		QVector3D			 V3 = V1->mVector3D - V2->mVector3D;

		pushvector3d( L, V3 );

		return( 1 );
	}

	return( 0 );
}

int LuaVector3D::luaLength(lua_State *L)
{
	UserData			*Vector3Data = checkvector3duserdata( L );
	const QVector3D		&V = Vector3Data->mVector3D;

	lua_pushnumber( L, V.length() );

	return( 1 );
}

int LuaVector3D::luaToArray( lua_State *L )
{
	UserData			*Vector3Data = checkvector3duserdata( L );
	const QVector3D		&V = Vector3Data->mVector3D;

	lua_newtable( L );

	lua_pushnumber( L, V.x() );
	lua_rawseti( L, -2, 1 );

	lua_pushnumber( L, V.y() );
	lua_rawseti( L, -2, 2 );

	lua_pushnumber( L, V.z() );
	lua_rawseti( L, -2, 3 );

	return( 1 );
}

int LuaVector3D::luaNormalize(lua_State *L)
{
	UserData			*Vector3Data = checkvector3duserdata( L );

	Vector3Data->mVector3D.normalize();

	return( 0 );
}

int LuaVector3D::luaNormalized(lua_State *L)
{
	UserData			*Vector3Data = checkvector3duserdata( L );

	pushvector3d( L, Vector3Data->mVector3D.normalized() );

	return( 1 );
}

int LuaVector3D::luaX(lua_State *L)
{
	UserData			*Vector3Data = checkvector3duserdata( L );
	const QVector3D		&V = Vector3Data->mVector3D;

	lua_pushnumber( L, V.x() );

	return( 1 );
}

int LuaVector3D::luaY(lua_State *L)
{
	UserData			*Vector3Data = checkvector3duserdata( L );
	const QVector3D		&V = Vector3Data->mVector3D;

	lua_pushnumber( L, V.y() );

	return( 1 );
}

int LuaVector3D::luaZ(lua_State *L)
{
	UserData			*Vector3Data = checkvector3duserdata( L );
	const QVector3D		&V = Vector3Data->mVector3D;

	lua_pushnumber( L, V.z() );

	return( 1 );
}

#endif
