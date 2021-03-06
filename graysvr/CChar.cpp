//
// CChar.cpp
// Copyright Menace Software (www.menasoft.com).
//  CChar is either an NPC or a Player.
//

#include "graysvr.h"	// predef header.
#include "CClient.h"

LPCTSTR const CChar::sm_szTrigName[CTRIG_QTY+1] =	// static
{
	"@AAAUNUSED",
	"@Attack",				// I am attacking someone (SRC)
	"@CallGuards",
	"@Click",				// I got clicked on by someone.
	"@Create",				// Newly created (not in the world yet)
	"@CreatePlayer",		//+A player char have just been added
	"@DClick",				// Someone has dclicked on me.
	"@Death",				//+I just got killed.
	"@DeathCorpse",
	"@Destroy",				//+I am nearly destroyed
	"@EnvironChange",		// my environment changed somehow (light,weather,season,region)
	"@ExpChange",			// EXP is going to change
	"@ExpLevelChange",		// Experience LEVEL is going to change
	"@GetHit",				// I just got hit.
	"@Hit",					// I just hit someone. (TARG)
	"@HitMiss",				// I just missed.
	"@HitTry",				// I am trying to hit someone. starting swing.
	"@Hunger",					//+Ready to update the food level

	// ITRIG_QTY
	"@itemClick",			// I clicked on an item
	"@itemCreate",			//?
	"@itemDamage",			//?
	"@itemDCLICK",			// I have dclicked item
	"@itemDestroy",			//+I am nearly destroyed
	"@itemDropOn_Char",		// I have been dropped on this char
	"@itemDropOn_Ground",	// I dropped an item on the ground
	"@itemDropOn_Item",		// I have been dropped on this item
	"@itemDropOn_Self",		// An item has been dropped on
	"@itemEQUIP",			// I have equipped an item
	"@itemEQUIPTEST",
	"@itemPICKUP_GROUND",
	"@itemPICKUP_PACK",		// picked up from inside some container.
	"@itemPICKUP_SELF",		// picked up from this container
	"@itemSPELL",			// cast some spell on the item.
	"@itemSTEP",			// stepped on an item
	"@itemTARGON_CANCEL",
	"@itemTARGON_CHAR",
	"@itemTARGON_GROUND",
	"@itemTARGON_ITEM",		// I am being combined with an item
	"@itemTimer",			//?
	"@itemToolTip",			// Did tool tips on an item
	"@itemUNEQUIP",			// i have unequipped (or try to unequip) an item

	"@Kill",				//+I have just killed someone
	"@LogIn",				// Client logs in
	"@LogOut",				// Client logs out (21)
	"@MurderDecay",			// One of my kills is gonna to be cleared
	"@MurderMark",			// I am gonna to be marked as a murderer

	"@NPCAcceptItem",		// (NPC only) i've been given an item i like (according to DESIRES)
	"@NPCActFight",
	"@NPCActFollow",			// (NPC only) following someone right now
	"@NPCHearGreeting",		// (NPC only) i have been spoken to for the first time. (no memory of previous hearing)
	"@NPCHearNeed",			// (NPC only) i heard someone mention something i need. (11)
	"@NPCHearUnknown",		//+(NPC only) I heard something i don't understand.
	"@NPCLookAtChar",		// (NPC only) look at a character
	"@NPCLookAtItem",		// (NPC only) look at a character
	"@NPCLostTeleport",		//+(NPC only) ready to teleport back to spawn
	"@NPCRefuseItem",		// (NPC only) i've been given an item i don't want.
	"@NPCRestock",			// (NPC only) 
	"@NPCSeeNewPlayer",		//+(NPC only) i see u for the first time. (in 20 minutes) (check memory time)
	"@NPCSeeWantItem",		// (NPC only) i see something good.

	"@PersonalSpace",	//+i just got stepped on.
	"@Profile",			// someone hit the profile button for me.
	"@ReceiveItem",		// I was just handed an item (Not yet checked if i want it)

	"@SeeCrime",		// I saw a crime

	// SKTRIG_QTY
	"@SkillAbort",
	"@SkillFail",
	"@SkillGain",
	"@SkillMakeItem",
	"@SkillSelect",
	"@SkillStart",
	"@SkillStroke",
	"@SkillSuccess",
	
	"@SpellBook",
	"@SpellCast",		//+Char is casting a spell.
	"@SpellEffect",		//+A spell just hit me.
	"@Step",			// Very expensive!
	"@StepStealth",				//+Made a step in stealth mode
	"@ToolTip",			// someone did tool tips on me.

	"@UserChatButton",
	"@UserExtCmd",
	"@UserExWalkLimit",
	"@UserMailBag",
	"@UserSkills",
	"@UserStats",
	"@UserVirtue",
	"@UserWarmode",

	// War mode ?
	 NULL,
};


/////////////////////////////////////////////////////////////////
// -CChar

CChar * CChar::CreateBasic( CREID_TYPE baseID ) // static
{
	// Create the "basic" NPC. Not NPC or player yet.
	// NOTE: NEVER return NULL
	return( new CChar( baseID ));
}

CChar * CChar::CreateNPC( CREID_TYPE baseID )	// static
{
	// Create an NPC
	// NOTE: NEVER return NULL
	CChar * pChar = CreateBasic( baseID );
	ASSERT(pChar);
	pChar->NPC_LoadScript(true);
	return( pChar );
}

CChar::CChar( CREID_TYPE baseID ) : CObjBase( false )
{
	g_Serv.StatInc( SERV_STAT_CHARS );	// Count created CChars.

	m_pArea = NULL;
	m_pParty = NULL;
	m_pClient = NULL;	// is the char a logged in player ?
	m_pPlayer = NULL;	// May even be an off-line player !
	m_pNPC	  = NULL;
	m_StatFlag = 0;

	if ( g_World.m_fSaveParity )
	{
		StatFlag_Set( STATF_SaveParity );	// It will get saved next time.
	}

	m_dirFace = DIR_SE;
	m_fonttype = FONT_NORMAL;

	m_defense = 0;

	// Initialize kuch's maxweight
	m_ModMaxWeight = 0;
	// Visual Range
	m_iVisualRange = UO_MAP_VIEW_SIZE;

	m_exp = 0;
	m_level = 0;

	m_atUnk.m_Arg1 = 0;
	m_atUnk.m_Arg2 = 0;
	m_atUnk.m_Arg3 = 0;

	m_timeLastRegen = m_timeCreate = CServTime::GetCurrentTime();
	m_timeLastHitsUpdate = m_timeLastRegen;
	m_fHitsUpdate = false;

	m_prev_Hue = HUE_DEFAULT;
	m_prev_id = CREID_INVALID;
	SetID( baseID );

	CCharBase* pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	SetName( pCharDef->GetTypeName());	// set the name in case there is a name template.

	Skill_Cleanup();

	g_World.m_uidLastNewChar = GetUID();	// for script access.

	int i=0;
	for ( ; i < STAT_QTY; i++ )
	{
		Stat_SetBase( (STAT_TYPE) i, 0 );
		Stat_SetMod(  (STAT_TYPE) i, 0 );
		Stat_SetVal(  (STAT_TYPE) i, 0 );
		Stat_SetMax(  (STAT_TYPE) i, 0 );
	}
	Stat_SetVal( STAT_FOOD, Stat_GetMax(STAT_FOOD) );

	for ( i=0;i<MAX_SKILL;i++)
	{
		m_Skill[i] = 0;
	}

	m_LocalLight = 0;
	m_fClimbUpdated = false;

	ASSERT( IsDisconnected());
}

CChar::~CChar() // Delete character
{
	DeletePrepare();	// remove me early so virtuals will work.
	if ( IsStatFlag( STATF_Ridden ))
	{
		CItem * pItem = Horse_GetMountItem();
		if ( pItem )
		{
			pItem->m_itFigurine.m_UID.InitUID();	// unlink it first.
			pItem->Delete();
		}
	}

	if ( IsClient())	// this should never happen.
	{
		ASSERT( m_pClient );
		m_pClient->m_fClosed	= true;
	}

	if ( m_pParty )
	{
		m_pParty->RemoveMember( GetUID(), (DWORD) GetUID() );
		m_pParty = NULL;
	}

	DeleteAll();		// remove me early so virtuals will work.
	ClearNPC();
	ClearPlayer();
	g_Serv.StatDec( SERV_STAT_CHARS );
}

void CChar::ClientDetach()
{
	// Client is detaching from this CChar.
	CancelAllTrades();
	if ( ! IsClient())
		return;

	if ( m_pParty && m_pParty->IsPartyMaster( this ))
	{
		// Party must disband if the master is logged out.
		m_pParty->Disband(GetUID());
		m_pParty = NULL;
	}

	// If this char is on a IT_SHIP then we need to stop the ship !
	if ( m_pArea && m_pArea->IsFlag( REGION_FLAG_SHIP ))
	{
		CItemMulti * pShipItem = dynamic_cast <CItemMulti *>( m_pArea->GetResourceID().ItemFind());
		if ( pShipItem )
		{
			pShipItem->Ship_Stop();
		}
	}

	//	Clear saved tags
	m_TagDefs.SetNum("NEXTPACKPICK", 0, true);

	CSector * pSector = GetTopSector();
	pSector->ClientDetach( this );
	m_pClient = NULL;
}

void CChar::ClientAttach( CClient * pClient )
{
	// Client is Attaching to this CChar.
	if ( GetClient() == pClient )
		return;
	DEBUG_CHECK( ! IsClient());
	DEBUG_CHECK( pClient->GetAccount());
	if ( ! SetPlayerAccount( pClient->GetAccount()))	// i now own this char.
		return;

	ASSERT(m_pPlayer);
	m_pPlayer->m_timeLastUsed = CServTime::GetCurrentTime();

	m_pClient = pClient;
	GetTopSector()->ClientAttach( this );
	FixClimbHeight();
}

void CChar::SetDisconnected()
{
	// Client logged out or NPC is dead.
	if ( IsClient())
	{
		GetClient()->m_fClosed	= true;
		return;
	}
	if ( m_pParty )
	{
		m_pParty->RemoveMember( GetUID(), (DWORD) GetUID() );
		m_pParty = NULL;
	}
	if ( IsDisconnected())
		return;
	DEBUG_CHECK( GetParent());
	RemoveFromView();	// Remove from views.
	// DEBUG_MSG(( "Disconnect '%s'" DEBUG_CR, (LPCTSTR) GetName()));
	MoveToRegion(NULL,false);
	GetTopSector()->m_Chars_Disconnect.InsertHead( this );
	DEBUG_CHECK( IsDisconnected());
}

int CChar::IsWeird() const
{
	// RETURN: invalid code.
	int iResultCode = CObjBase::IsWeird();
	if ( iResultCode )
		return iResultCode;

	if ( IsDisconnected())
	{
		if ( ! GetTopSector()->IsCharDisconnectedIn( this ))
		{
			iResultCode = 0x1102;
			return iResultCode;
		}
		if ( m_pNPC )
		{
			if ( IsStatFlag( STATF_Ridden ))
			{
				if ( Skill_GetActive() != NPCACT_RIDDEN )
				{
					iResultCode = 0x1103;
					return iResultCode;
				}

				// Make sure we are still linked back to the world.
				CItem * pItem = Horse_GetMountItem();
				if ( pItem == NULL )
				{
					iResultCode = 0x1104;
					return iResultCode;
				}
				if ( pItem->m_itFigurine.m_UID != GetUID())
				{
					iResultCode = 0x1105;
					return iResultCode;
				}
			}
			else
			{
				if ( ! IsStatFlag( STATF_DEAD ))
				{
					iResultCode = 0x1106;
					return iResultCode;
				}
			}
		}
	}

	if ( ! m_pPlayer && ! m_pNPC )
	{
		iResultCode = 0x1107;
		return iResultCode;
	}

	if ( ! GetTopPoint().IsValidPoint())
	{
		iResultCode = 0x1108;
		return iResultCode;
	}

	return( 0 );
}

int CChar::FixWeirdness()
{
	// Clean up weird flags.
	// fix Weirdness.
	// NOTE:
	//  Deleting a player char is VERY BAD ! Be careful !
	//
	// RETURN: false = i can't fix this.

	int iResultCode = CObjBase::IsWeird();
	if ( iResultCode )
		// Not recoverable - must try to delete the object.
		return( iResultCode );

	// NOTE: Stats and skills may go negative temporarily.

	CCharBase * pCharDef = Char_GetDef();

	// Make sure my flags are good.

	if ( IsStatFlag( STATF_Insubstantial ))
	{
		if ( ! IsStatFlag( STATF_DEAD ) && ! IsPriv(PRIV_GM) && GetPrivLevel() < PLEVEL_Counsel )
		{
			StatFlag_Clear( STATF_Insubstantial );
		}
	}
	if ( IsStatFlag( STATF_HasShield ))
	{
		CItem * pShield = LayerFind( LAYER_HAND2 );
		if ( pShield == NULL )
		{
			StatFlag_Clear( STATF_HasShield );
		}
	}
	if ( IsStatFlag( STATF_OnHorse ))
	{
		CItem * pHorse = LayerFind( LAYER_HORSE );
		if ( pHorse == NULL )
		{
			StatFlag_Clear( STATF_OnHorse );
		}
	}
	if ( IsStatFlag( STATF_Spawned ))
	{
		CItemMemory * pMemory = Memory_FindTypes( MEMORY_ISPAWNED );
		if ( pMemory == NULL )
		{
			StatFlag_Clear( STATF_Spawned );
		}
	}
	if ( IsStatFlag( STATF_Pet ))
	{
		CItemMemory * pMemory = Memory_FindTypes( MEMORY_IPET );
		if ( pMemory == NULL )
		{
			StatFlag_Clear( STATF_Pet );
		}
	}
	if ( IsStatFlag( STATF_Ridden ))
	{
		// Move the ridden creature to the same location as it's rider.
		if ( m_pPlayer || ! IsDisconnected())
		{
			StatFlag_Clear( STATF_Ridden );
		}
		else
		{
			if ( Skill_GetActive() != NPCACT_RIDDEN )
			{
				iResultCode = 0x1203;
				return iResultCode;
			}
			CItem * pFigurine = Horse_GetMountItem();
			if ( pFigurine == NULL )
			{
				iResultCode = 0x1204;
				return iResultCode;
			}
			CPointMap pt = pFigurine->GetTopLevelObj()->GetTopPoint();
			if ( pt != GetTopPoint())
			{
				MoveToChar( pt );
				SetDisconnected();
			}
		}
	}
	if ( IsStatFlag( STATF_Criminal ))
	{
		// make sure we have a criminal flag timer ?
	}

	if ( ! IsIndividualName() && pCharDef->GetTypeName()[0] == '#' )
	{
		SetName( pCharDef->GetTypeName());
	}
	if ( ! CCharBase::IsValidDispID( GetID()) && CCharBase::IsHumanID( m_prev_id ))
	{
		// This is strange. (has human body)
		m_prev_id = GetID();
	}

	if ( m_pPlayer )	// Player char.
	{
		DEBUG_CHECK( ! IsStatFlag( STATF_Pet | STATF_Ridden | STATF_Spawned ));
		Memory_ClearTypes(MEMORY_ISPAWNED|MEMORY_IPET);
		StatFlag_Clear( STATF_Ridden );

		if ( m_pPlayer->GetSkillClass() == NULL )	// this hsould never happen.
		{
			m_pPlayer->SetSkillClass( this, RESOURCE_ID( RES_SKILLCLASS ));
			ASSERT(m_pPlayer->GetSkillClass());
		}

		// Make sure players don't get ridiculous stats.
		//		m_iOverSkillMultiply disables this check if set to < 1
		if (( GetPrivLevel() <= PLEVEL_Player ) && ( g_Cfg.m_iOverSkillMultiply > 0 ))
		{
			for ( int i=0; i<MAX_SKILL; i++ )
			{
				int iSkillMax = Skill_GetMax( (SKILL_TYPE)i );
				int iSkillVal = Skill_GetBase( (SKILL_TYPE)i );
				if ( iSkillVal < 0 )
					Skill_SetBase( (SKILL_TYPE)i, 0 );
				if ( iSkillVal > iSkillMax * g_Cfg.m_iOverSkillMultiply )
					Skill_SetBase( (SKILL_TYPE)i, iSkillMax );
			}

			// ??? What if magically enhanced !!!
			if ( IsHuman() &&
				GetPrivLevel() < PLEVEL_Counsel &&
				! IsStatFlag( STATF_Polymorph ))
			{
				for ( int j=STAT_STR; j<STAT_BASE_QTY; j++ )
				{
					int iStatMax = Stat_GetLimit((STAT_TYPE)j);
					if ( Stat_GetAdjusted((STAT_TYPE)j) > iStatMax*g_Cfg.m_iOverSkillMultiply )
					{
						Stat_SetBase((STAT_TYPE)j, iStatMax );
					}
				}
			}
		}
	}
	else
	{
		if ( ! m_pNPC )
		{
			iResultCode = 0x1205;
			return iResultCode;
		}

		if ( ! strcmp( GetName(), "ship" ))
		{
			// version .37 cleanup code.
			iResultCode = 0x1206;
			return iResultCode;
		}

		// An NPC. Don't keep track of unused skills.
		for ( int i=0; i<MAX_SKILL; i++ )
		{
			if ( m_Skill[i] && m_Skill[i] <= 10 )
				Skill_SetBase( (SKILL_TYPE)i, 0 );
		}
	}

	if ( GetTimerAdjusted() > 60*60 )
	{
		// unreasonably long for a char?
		SetTimeout(1);
	}

	// FixWeight();

	return IsWeird();
}

void CChar::CreateNewCharCheck()
{
	// Creating a new char. (Not loading from save file) MAke sure things are set to reasonable values.
	m_prev_id = GetID();
	m_prev_Hue = GetHue();

	Stat_SetVal(STAT_STR, Stat_GetMax(STAT_STR));
	Stat_SetVal(STAT_DEX, Stat_GetMax(STAT_DEX));
	Stat_SetVal(STAT_INT, Stat_GetMax(STAT_INT));

	if ( !m_pPlayer )	// need a starting brain tick.
	{
		//	auto-set EXP/LEVEL level
		if ( g_Cfg.m_bExperienceSystem && g_Cfg.m_iExperienceMode&EXP_MODE_AUTOSET_EXP )
		{
			if ( !m_exp )
				m_exp = (Stat_GetMax(STAT_STR) + Stat_GetMax(STAT_DEX)/2 + Stat_GetMax(STAT_INT))/10;

			if ( !m_level && g_Cfg.m_bLevelSystem && ( m_exp > g_Cfg.m_iLevelNextAt ))
				ChangeExperience();
		}

		SetTimeout(1);
	}
}

bool CChar::ReadScriptTrig( CCharBase * pCharDef, CTRIG_TYPE trig )
{
	if ( pCharDef == NULL )
		return( false );
	if ( ! pCharDef->HasTrigger( trig ))
		return( false );
	CResourceLock s;
	if ( ! pCharDef->ResourceLock(s))
		return( false );
	if ( ! OnTriggerFind( s, sm_szTrigName[trig] ))
		return( false );
	return( ReadScript( s ));
}

bool CChar::ReadScript( CResourceLock &s )
{
	// If this is a regen they will have a pack already.
	// fRestock = this is a vendor restock.
	// fNewbie = newbiize all the stuff we get.
	// NOTE:
	//  It would be nice to know the total $ value of items created here !
	// RETURN:
	//  true = default return. (mostly ignored).
	bool fFullInterp		= false;
	bool fIgnoreAttributes	= false;

	CItem * pItem = NULL;
	while ( s.ReadKeyParse())
	{
		if ( s.IsKeyHead( "ON", 2 ))
			break;

		int iCmd = FindTableSorted( s.GetKey(), CItem::sm_szTemplateTable, COUNTOF( CItem::sm_szTemplateTable )-1 );
		switch ( iCmd )
		{
		case ITC_FULLINTERP:
			{
				LPCTSTR		pszArgs	= s.GetArgStr();
				GETNONWHITESPACE( pszArgs );
				if ( !*pszArgs )
					fFullInterp	= true;
				else
					fFullInterp	= s.GetArgVal();
				continue;
			}
		case ITC_NEWBIESWAP:
			{
				if ( !pItem || fIgnoreAttributes )
					continue;

				if ( pItem->IsAttr( ATTR_NEWBIE ) )
				{
					if ( Calc_GetRandVal( s.GetArgVal() ) == 0 )
						pItem->ClrAttr(ATTR_NEWBIE);
				}				
				else
				{
					if ( Calc_GetRandVal( s.GetArgVal() ) == 0 )
						pItem->SetAttr(ATTR_NEWBIE);
				}				
				continue;
			}
		case ITC_ITEM:
		case ITC_CONTAINER:
		case ITC_ITEMNEWBIE:
		{
			// Possible loot/equipped item.
			fIgnoreAttributes = true;
	
			if ( IsStatFlag( STATF_Conjured ) && iCmd != ITC_ITEMNEWBIE ) // This check is not needed.
				break; // conjured creates have no loot.
	
			pItem = CItem::CreateHeader( s.GetArgRaw(), this, iCmd == ITC_ITEMNEWBIE );
			if ( pItem == NULL )
				continue;

			if ( iCmd == ITC_ITEMNEWBIE )
			{
				pItem->SetAttr(ATTR_NEWBIE);
			}

			if ( pItem->IsItemInContainer() || pItem->IsItemEquipped())
				fIgnoreAttributes = false;
			continue;
		}
		case ITC_BUY:
		case ITC_SELL:
		{
			fIgnoreAttributes = true;

			CItemContainer * pCont = GetBank((iCmd == ITC_SELL) ? LAYER_VENDOR_STOCK : LAYER_VENDOR_BUYS );
			if ( pCont == NULL )
			{
				DEBUG_ERR(( "NPC '%s', is not a vendor!" DEBUG_CR, (LPCTSTR) GetResourceName()));
				continue;
			}
			pItem = CItem::CreateHeader( s.GetArgRaw(), pCont, false );
			if ( pItem == NULL )
				continue;

			if ( pItem->IsItemInContainer())
			{
				fIgnoreAttributes = false;
				pItem->SetContainedLayer( pItem->GetAmount());	// set the Restock amount.
			}
			continue;
		}
		}

		if ( fIgnoreAttributes )	// some item creation failure.
			continue;

		if ( pItem != NULL )
		{
			if ( fFullInterp )	// Modify the item.
				pItem->r_Verb( s, &g_Serv );
			else
				pItem->r_LoadVal( s );
		}
		else
		{
			TRIGRET_TYPE tRet = OnTriggerRun( s, TRIGRUN_SINGLE_EXEC, &g_Serv, NULL );
			if ( (tRet == TRIGRET_RET_FALSE) && fFullInterp )
				;
			else if ( tRet != TRIGRET_RET_DEFAULT )
			{
				return (tRet == TRIGRET_RET_FALSE);
			}
		}
	}

	return( true );
}

void CChar::NPC_LoadScript( bool fRestock )
{
	// Create an NPC from script.
	if ( m_pNPC == NULL )
	{
		// Set a default brian type til we get the real one from scripts.
		SetNPCBrain( GetNPCBrain( true ) );	// should have a default brain. watch out for override vendor.
	}

	CCharBase * pCharDef = Char_GetDef();
	ReadScriptTrig( pCharDef, CTRIG_Create );
   //Add real on=@create here!!!

	if ( fRestock )
	{

		// OnTrigger( CTRIG_NPCRestock, &g_Serv );
		ReadScriptTrig( pCharDef, CTRIG_NPCRestock );
	}

	if ( NPC_IsVendor())
	{
		// Restock it now so it can buy/sell stuff immediately
		NPC_Vendor_Restock( 15*60 );
	}

	CreateNewCharCheck();
}

void CChar::OnWeightChange( int iChange )
{
	CContainer::OnWeightChange( iChange );
	UpdateStatsFlag();
}

bool CChar::SetName( LPCTSTR pszName )
{
	return SetNamePool( pszName );
}

void CChar::SetID( CREID_TYPE id )
{
	// Just set the base id and not the actual display id.
	// NOTE: Never return NULL

	CCharBase * pCharDef = CCharBase::FindCharBase( id );
	if ( pCharDef == NULL )
	{
		if ( id != -1 && id != CREID_INVALID )
		{
			DEBUG_ERR(( "Create Invalid Char 0%x" DEBUG_CR, id ));
		}
		pCharDef = Char_GetDef();
		if ( pCharDef != NULL )
			return;
		id = (CREID_TYPE) g_Cfg.ResourceGetIndexType( RES_CHARDEF, "DEFAULTCHAR" );
		if ( id < 0 )
		{
			id = CREID_OGRE;
		}
		pCharDef = CCharBase::FindCharBase(id);
	}

	if ( pCharDef == Char_GetDef())
		return;

	m_BaseRef.SetRef( pCharDef );

	if ( m_prev_id == CREID_INVALID )
	{
		m_prev_id = GetID();
	}

	if ( GetNPCBrain() != NPCBRAIN_HUMAN )
	{
		// Transfom to non-human (if they ever where human)
		// can't ride a horse in this form.
		Horse_UnMount();
		UnEquipAllItems(); 		// unequip all items.
	}
}

void CChar::InitPlayer( const CEvent * pBin, CClient * pClient )
{
	// Create a brand new Player char.
	ASSERT(pClient);
	ASSERT(pBin);

	SetID( (CREID_TYPE) g_Cfg.ResourceGetIndexType( RES_CHARDEF, ( pBin->Create.m_sex == 0 ) ? "c_MAN" : "c_WOMAN" ));

	bool	bNameIsAccepted = true;	// Is the name acceptable?

	if ( g_Cfg.IsObscene(pBin->Create.m_charname) || Str_Check(pBin->Create.m_charname) ||
		!strnicmp(pBin->Create.m_charname,"lord ", 5) || !strnicmp(pBin->Create.m_charname,"lady ", 5) ||
		!strnicmp(pBin->Create.m_charname,"seer ", 5) || !strnicmp(pBin->Create.m_charname,"gm ", 3) ||
		!strnicmp(pBin->Create.m_charname,"admin ", 6) || !strnicmp(pBin->Create.m_charname,"counselor ", 10))
		bNameIsAccepted = false;

	if ( bNameIsAccepted )
	{
		CScriptTriggerArgs args;
		args.m_s1 = pBin->Create.m_charname;
		args.m_pO1 = this;
		if ( OnTrigger("@Rename", this, &args) == TRIGRET_RET_TRUE ) bNameIsAccepted = false;
	}

	if ( bNameIsAccepted ) SetName(pBin->Create.m_charname);
	else
	{
		g_Log.Event( LOGL_WARN|LOGM_ACCOUNTS,
			"%x:Unacceptable name '%s' for account '%s'" DEBUG_CR,
			pClient->m_Socket.GetSocket(), (LPCTSTR) pBin->Create.m_charname, (LPCTSTR) pClient->GetAccount()->GetName() );
		SetNamePool( ( pBin->Create.m_sex == 0 ) ? "#HUMANMALE" : "#HUMANFEMALE" );
	}

	HUE_TYPE wHue;
	wHue = pBin->Create.m_wSkinHue | HUE_UNDERWEAR;
	if ( wHue < (HUE_UNDERWEAR|HUE_SKIN_LOW) || wHue > (HUE_UNDERWEAR|HUE_SKIN_HIGH))
	{
		wHue = HUE_UNDERWEAR|HUE_SKIN_LOW;
	}
	SetHue( wHue );
	m_fonttype = FONT_NORMAL;

	int iStartLoc = pBin->Create.m_startloc-1;
	if ( ! g_Cfg.m_StartDefs.IsValidIndex( iStartLoc ))
		iStartLoc = 0;
	m_ptHome = g_Cfg.m_StartDefs[iStartLoc]->m_pt;

	if ( ! m_ptHome.IsValidPoint())
	{
		if ( g_Cfg.m_StartDefs.GetCount())
		{
			m_ptHome = g_Cfg.m_StartDefs[0]->m_pt;
		}
		DEBUG_ERR(( "Invalid start location for character!" DEBUG_CR ));
	}

	SetUnkPoint( m_ptHome );	// Don't actaully put me in the world yet.

	// randomize the skills first.
	int i = 0;
	for ( ; i < MAX_SKILL; i++ )
	{
		Skill_SetBase( (SKILL_TYPE)i, Calc_GetRandVal( g_Cfg.m_iMaxBaseSkill ));
	}

	if ( pBin->Create.m_str + pBin->Create.m_dex + pBin->Create.m_int > 80 )
	{
		// ! Cheater !
		Stat_SetBase( STAT_STR, 10 );
		Stat_SetBase( STAT_DEX, 10 );
		Stat_SetBase( STAT_INT, 10 );
		g_Log.Event( LOGL_WARN|LOGM_CHEAT, 
			"%x:Cheater '%s' is submitting hacked char stats" DEBUG_CR,
			pClient->m_Socket.GetSocket(), (LPCTSTR) pClient->GetAccount()->GetName());
	}
	else
	{
		Stat_SetBase( STAT_STR, pBin->Create.m_str + 1 );
		Stat_SetBase( STAT_DEX, pBin->Create.m_dex + 1 );
		Stat_SetBase( STAT_INT, pBin->Create.m_int + 1 );
	}

	if ( pBin->Create.m_val1 > 50 ||
		pBin->Create.m_val2 > 50 ||
		pBin->Create.m_val3 > 50 ||
		pBin->Create.m_val1 + pBin->Create.m_val2 + pBin->Create.m_val3 > 101 )
	{
		// ! Cheater !
		g_Log.Event( LOGL_WARN|LOGM_CHEAT,
			"%x:Cheater '%s' is submitting hacked char skills" DEBUG_CR,
			pClient->m_Socket.GetSocket(), (LPCTSTR) pClient->GetAccount()->GetName());
	}
	else
	{
		if ( IsSkillBase((SKILL_TYPE) pBin->Create.m_skill1))
			Skill_SetBase( (SKILL_TYPE) pBin->Create.m_skill1, pBin->Create.m_val1*10 );
		if ( IsSkillBase((SKILL_TYPE) pBin->Create.m_skill2))
			Skill_SetBase( (SKILL_TYPE) pBin->Create.m_skill2, pBin->Create.m_val2*10 );
		if ( IsSkillBase((SKILL_TYPE) pBin->Create.m_skill3))
			Skill_SetBase( (SKILL_TYPE) pBin->Create.m_skill3, pBin->Create.m_val3*10 );
	}

	ITEMID_TYPE id = (ITEMID_TYPE)(WORD) pBin->Create.m_hairid;
	if ( id )
	{
		CItem * pHair = CItem::CreateScript( id, this );
		ASSERT(pHair);
		if ( ! pHair->IsType(IT_HAIR))
		{
			// Cheater !
			pHair->Delete();
		}
		else
		{
			wHue = pBin->Create.m_hairHue;
			if ( wHue<HUE_HAIR_LOW || wHue > HUE_HAIR_HIGH )
			{
				wHue = HUE_HAIR_LOW;
			}
			pHair->SetHue( wHue );
			pHair->SetAttr(ATTR_NEWBIE|ATTR_MOVE_NEVER);
			LayerAdd( pHair );	// add content
		}
	}

	id = (ITEMID_TYPE)(WORD) pBin->Create.m_beardid;
	if ( id )
	{
		CItem * pBeard = CItem::CreateScript( id, this );
		ASSERT(pBeard);
		if ( ! pBeard->IsType(IT_BEARD))
		{
			// Cheater !
			pBeard->Delete();
		}
		else
		{
			wHue = pBin->Create.m_beardHue;
			if ( wHue < HUE_HAIR_LOW || wHue > HUE_HAIR_HIGH )
			{
				wHue = HUE_HAIR_LOW;
			}
			pBeard->SetHue( wHue );
			pBeard->SetAttr(ATTR_NEWBIE|ATTR_MOVE_NEVER);
			LayerAdd( pBeard );	// add content
		}
	}

	// Create the bank box.
	CItemContainer * pBankBox = GetBank( LAYER_BANKBOX );
	// Create the pack.
	CItemContainer * pPack = GetPackSafe();

	// Get special equip for the starting skills.
	for ( i=0; i<4; i++ )
	{
		int iSkill;
		if (i)
		{
			switch ( i )
			{
			case 1: iSkill = pBin->Create.m_skill1; break;
			case 2: iSkill = pBin->Create.m_skill2; break;
			case 3: iSkill = pBin->Create.m_skill3; break;
			}
		}
		else
		{
			iSkill = ( pBin->Create.m_sex == 0 ) ? RES_NEWBIE_MALE_DEFAULT : RES_NEWBIE_FEMALE_DEFAULT;
		}

		CResourceLock s;
		if ( ! g_Cfg.ResourceLock( s, RESOURCE_ID( RES_NEWBIE, iSkill )))
			continue;
		ReadScript( s );
	}

	// if ( pClient->m_Crypt.GetClientVer() >= 0x126000 )
	{
		HUE_TYPE wHue = pBin->Create.m_shirtHue;
		CItem * pLayer = LayerFind( LAYER_SHIRT );
		if ( pLayer )
		{
         		if ( wHue<HUE_BLUE_LOW || wHue>HUE_DYE_HIGH )
				wHue = HUE_DYE_HIGH;
			pLayer->SetHue( wHue );
		}
		wHue = pBin->Create.m_pantsHue;
		pLayer = LayerFind( LAYER_PANTS );
		if ( pLayer )
		{
         		if ( wHue<HUE_BLUE_LOW || wHue>HUE_DYE_HIGH )
				wHue = HUE_DYE_HIGH;
			pLayer->SetHue( wHue );
		}
	}

	CreateNewCharCheck();
}

enum CHR_TYPE
{
	CHR_ACCOUNT,
	CHR_ACT,
	CHR_FINDLAYER,
	CHR_MEMORYFIND,
	CHR_MEMORYFINDTYPE,
	CHR_OWNER,
	CHR_REGION,
	CHR_SPAWNITEM,
	CHR_WEAPON,
	CHR_QTY,
};

LPCTSTR const CChar::sm_szRefKeys[CHR_QTY+1] =
{
	"ACCOUNT",
	"ACT",
	"FINDLAYER",
	"MEMORYFIND",
	"MEMORYFINDTYPE",
	"OWNER",
	"REGION",
	"SPAWNITEM",
	"WEAPON",
	NULL,
};

bool CChar::r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef )
{
	int i = FindTableHeadSorted( pszKey, sm_szRefKeys, COUNTOF(sm_szRefKeys)-1 );
	if ( i >= 0 )
	{
		pszKey += strlen( sm_szRefKeys[i] );
		SKIP_SEPERATORS(pszKey);
		switch (i)
		{
		case CHR_ACCOUNT:
			if ( pszKey[-1] != '.' )	// only used as a ref !
				break;
			if ( m_pPlayer == NULL )
			{
				pRef = NULL;
			}
			else
			{
				pRef = m_pPlayer->GetAccount();
			}
			return( true );
		case CHR_ACT:
			if ( pszKey[-1] != '.' )	// only used as a ref !
				break;
			pRef = m_Act_Targ.ObjFind();
			return( true );
		case CHR_FINDLAYER:	// Find equipped layers.
			pRef = LayerFind( (LAYER_TYPE) Exp_GetSingle( pszKey ));
			SKIP_SEPERATORS(pszKey);
			return( true );
		case CHR_MEMORYFINDTYPE:	// FInd a type of memory.
			pRef = Memory_FindTypes( Exp_GetSingle( pszKey ));
			SKIP_SEPERATORS(pszKey);
			return( true );
		case CHR_MEMORYFIND:	// Find a memory of a UID
			pRef = Memory_FindObj( (CGrayUID) Exp_GetSingle( pszKey ));
			SKIP_SEPERATORS(pszKey);
			return( true );
		case CHR_OWNER:
			pRef	= NPC_PetGetOwner();
			return( true );
		case CHR_SPAWNITEM:
			{
				CItemMemory * pMemory = Memory_FindTypes( MEMORY_ISPAWNED );
				if ( !pMemory )
					pRef	= NULL;
				else
					pRef	= pMemory->m_uidLink.ItemFind();
				return( true );
			}
		case CHR_WEAPON:
			{
				pRef	= m_uidWeapon.ObjFind();
				return( true );
			}
		case CHR_REGION:
			pRef = m_pArea;
			return( true );
		}
	}

	if ( r_GetRefContainer( pszKey, pRef ))
	{
		return( true );
	}

	return( CObjBase::r_GetRef( pszKey, pRef ));
}

enum CHC_TYPE
{
	CHC_AC = 0,
	CHC_ACCOUNT,
	CHC_ACT,
	CHC_ACTARG1,
	CHC_ACTARG2,
	CHC_ACTARG3,
	CHC_ACTDIFF,
	CHC_ACTION,
	CHC_AGE,
	CHC_AR,
	CHC_BANKBALANCE,
	CHC_BODY,
	CHC_CANCAST,
	CHC_CANMAKE,
	CHC_CANMAKESKILL,
	CHC_CANMOVE,
	CHC_CREATE,
	CHC_DIR,
	CHC_DISMOUNT,
	CHC_DISPIDDEC,
	CHC_EMOTEACT,
	CHC_EXP,
	CHC_FAME,
	CHC_FLAGS,
	CHC_FONT,
	CHC_FOOD,
	CHC_GUILDABBREV,
	CHC_HITPOINTS,
	CHC_HITS,
	CHC_HOME,
	CHC_ID,
	CHC_ISGM,
	CHC_ISMYPET,
	CHC_ISONLINE,
	CHC_ISPLAYER,
	CHC_ISSTUCK,
	CHC_ISTEVENT,
	CHC_ISVENDOR,
	CHC_KARMA,
	CHC_LEVEL,
	CHC_LIGHT,
	CHC_MANA,
	CHC_MAXHITS,
	CHC_MAXMANA,
	CHC_MAXSTAM,
	CHC_MAXWEIGHT,
	CHC_MEMORY,
	CHC_MODMAXWEIGHT,
	CHC_MOUNT,
	CHC_MOVE,
	CHC_NAME,
	CHC_NIGHTSIGHT,
	CHC_NOTOGETFLAG,
	CHC_NPC,
	CHC_OBODY,
	CHC_OSKIN,
	CHC_P,
	CHC_RANGE,
	CHC_SEX,
	CHC_SKILLBEST,
	CHC_SKILLCHECK,
	CHC_SKILLTOTAL,
	CHC_STAM,
	CHC_STAMINA,
	CHC_STONE,
	CHC_TITLE,
	CHC_TOWNABBREV,
	CHC_VISUALRANGE,
	CHC_QTY,
};

LPCTSTR const CChar::sm_szLoadKeys[CHC_QTY+1] =
{
	"ac",
	"ACCOUNT",
	"ACT",
	"ACTARG1",
	"ACTARG2",
	"ACTARG3",
	"ACTDIFF",
	"ACTION",
	"AGE",
	"AR",
	"bankbalance",
	"BODY",
	"CANCAST",
	"CANMAKE",
	"CANMAKESKILL",
	"CANMOVE",
	"CREATE",
	"DIR",
	"DISMOUNT",
	"dispiddec",	// for properties dialog.
	"EMOTEACT",
	"EXP",
	"FAME",
	"FLAGS",
	"FONT",
	"FOOD",
	"GUILDABBREV",
	"HITPOINTS",
	"HITS",
	"HOME",
	"ID",
	"ISGM",
	"ISMYPET",
	"ISONLINE",
	"ISPLAYER",
	"ISSTUCK",
	"ISTEVENT",
	"ISVENDOR",
	"KARMA",
	"LEVEL",
	"LIGHT",
	"MANA",
	"MAXHITS",
	"MAXMANA",
	"MAXSTAM",
	"MAXWEIGHT",
	"MEMORY",
	"MODMAXWEIGHT",
	"MOUNT",
	"MOVE",
	"NAME",
	"NIGHTSIGHT",
	"NOTOGETFLAG",
	"NPC",
	"OBODY",
	"OSKIN",
	"P",
	"RANGE",
	"SEX",
	"SKILLBEST",
	"SKILLCHECK",
	"SKILLTOTAL",
	"STAM",
	"STAMINA",
	"STONE",
	"TITLE",
	"TOWNABBREV",
	"VISUALRANGE",
	NULL,
};

void CChar::r_DumpLoadKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szLoadKeys);
	CObjBase::r_DumpLoadKeys(pSrc);
}
void CChar::r_DumpVerbKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szVerbKeys);
	CObjBase::r_DumpVerbKeys(pSrc);
}



bool CChar::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
{
	EXC_TRY(("r_WriteVal('%s',,%x)", pszKey, pSrc));
	static LPCTSTR const sm_szFameGroups[] =	// display only type stuff.
	{
		"ANONYMOUS",
		"FAMOUS",
		"INFAMOUS",	// get rid of this in the future.
		"KNOWN",
		"OUTLAW",
		NULL,
	};
	static const CValStr sm_KarmaTitles[] =
	{
		"WICKED", INT_MIN,				// -10000 to -6001
		"BELLIGERENT", -6000,			// -6000 to -2001
		"NEUTRAL", -2000,				// -2000 to  2000
		"KINDLY", 2001,				// 2001 to  6000
		"GOODHEARTED", 6001,		// 6001 to 10000
		NULL, INT_MAX,
	};

	if ( IsClient())
	{
		if ( GetClient()->r_WriteVal( pszKey, sVal, pSrc ))
			return( true );
	}

	CCharBase * pCharDef = Char_GetDef();
	ASSERT(pCharDef);
	CChar * pCharSrc = pSrc->GetChar();

	CHC_TYPE iKeyNum = (CHC_TYPE) FindTableHeadSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 );
	if ( iKeyNum < 0 )
	{
do_default:
		if ( m_pPlayer )
		{
			if ( m_pPlayer->r_WriteVal( this, pszKey, sVal ))
				return( true );
		}
		if ( m_pNPC )
		{
			if ( m_pNPC->r_WriteVal( this, pszKey, sVal ))
				return( true );
		}

		if ( r_WriteValContainer( pszKey, sVal ))
		{
			return( true );
		}

		// special write values
		int i;

		// Adjusted stats
		i = g_Cfg.FindStatKey( pszKey );
		if ( i >= 0 )
		{
			sVal.FormatVal( Stat_GetAdjusted( (STAT_TYPE) i));
			return( true );
		}

		if ( !strnicmp( pszKey, "O", 1 ) )
		{
			i = g_Cfg.FindStatKey( pszKey+1 );
			if ( i >= 0 )
			{
				sVal.FormatVal( Stat_GetBase( (STAT_TYPE) i));
				return( true );
			}
		}
		
		if ( !strnicmp( pszKey, "MOD", 3 ) )
		{
			i = g_Cfg.FindStatKey( pszKey+3 );
			if ( i >= 0 )
			{
				sVal.FormatVal( Stat_GetMod( (STAT_TYPE) i));
				return( true );
			}
		}
		
		i = g_Cfg.FindSkillKey( pszKey );
		if ( IsSkillBase((SKILL_TYPE)i))
		{
			// Check some skill name.
			short iVal = Skill_GetBase( (SKILL_TYPE) i );
			sVal.Format( "%i.%i", iVal/10, iVal%10 );
			return( true );
		}

		return( CObjBase::r_WriteVal( pszKey, sVal, pSrc ));
	}

	switch ( iKeyNum )
	{
	case CHC_RANGE:
		sVal.FormatVal( CalcFightRange( m_uidWeapon.ItemFind() ) );
		return true;
	case CHC_FAME:
		// How much respect do i give this person ?
		// Fame is never negative !
		if ( pszKey[4] != '.' )
			goto do_default;
		pszKey += 5;
		{
		int iFame = Stat_GetAdjusted(STAT_FAME);
		int iKarma = Stat_GetAdjusted(STAT_KARMA);

		switch ( FindTableSorted( pszKey, sm_szFameGroups, COUNTOF( sm_szFameGroups )-1 ))
		{
		case 0: // "ANONYMOUS"
			iFame = ( iFame < 2000 );
			break;
		case 1: // "FAMOUS"
			iFame = ( iFame > 6000 );
			break;
		case 2: // "INFAMOUS"
			iFame = ( iFame > 6000 && iKarma <= -6000 );
			break;
		case 3: // "KNOWN"
			iFame = ( iFame > 2000 );
			break;
		case 4: // "OUTLAW"
			iFame = ( iFame > 2000 && iKarma <= -2000 );
			break;
		}
		sVal = iFame ? "1" : "0";
		}
		return( true );

	case CHC_SKILLCHECK:	// odd way to get skills checking into the triggers.
		pszKey += 10;
		SKIP_SEPERATORS(pszKey);
		{
			TCHAR * ppArgs[2];
			Str_ParseCmds( (TCHAR*) pszKey, ppArgs, COUNTOF( ppArgs ));
			SKILL_TYPE iSkill = g_Cfg.FindSkillKey( ppArgs[0] );
			if ( iSkill == SKILL_NONE )
				return( false );
			sVal.FormatVal( Skill_CheckSuccess( iSkill, Exp_GetVal( ppArgs[1] )));
		}
		return( true );
	case CHC_SKILLBEST:
		// Get the top skill.
		pszKey += 9;
		{
			int iRank = 0;
			if ( *pszKey == '.' )
			{
				SKIP_SEPERATORS(pszKey);
				iRank = Exp_GetSingle(pszKey);
			}
			sVal.FormatVal(Skill_GetBest(iRank));
		}
		return true;
	case CHC_SEX:	// <SEX milord/milady>	sep chars are :,/
		pszKey += 4;
		SKIP_SEPERATORS(pszKey);
		{
		TCHAR * ppArgs[2];
		Str_ParseCmds( (TCHAR*) pszKey, ppArgs, COUNTOF(ppArgs), ":,/" );
		sVal = ( pCharDef->IsFemale()) ? ppArgs[1] : ppArgs[0];
		}
		return( true );
	case CHC_KARMA:
		// What do i think of this person.
		if ( pszKey[5] != '.' )
			goto do_default;
		pszKey += 6;
		sVal = ( ! strcmpi( pszKey, sm_KarmaTitles->FindName( Stat_GetAdjusted(STAT_KARMA)))) ? "1" : "0";
		return( true );
	case CHC_AR:
	case CHC_AC:
		sVal.FormatVal( m_defense + pCharDef->m_defense );
		return( true );
	case CHC_AGE:
		sVal.FormatVal(( - g_World.GetTimeDiff(m_timeCreate)) / TICK_PER_SEC );
		return( true );
	case CHC_BANKBALANCE:
		sVal.FormatVal( GetBank()->ContentCount( RESOURCE_ID(RES_TYPEDEF,IT_GOLD)));
		return true;
	case CHC_CANCAST:
		{
			pszKey += 7;
			SPELL_TYPE spell = (SPELL_TYPE) g_Cfg.ResourceGetIndexType( RES_SPELL, pszKey );
			sVal.FormatVal( Spell_CanCast( spell, true, this, false ));
		}
		return true;
	case CHC_CANMAKE:
		{
			// use m_Act_Targ ?
			pszKey += 7;
			ITEMID_TYPE id = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, pszKey );
			sVal.FormatVal( Skill_MakeItem( id,	UID_CLEAR, SKTRIG_SELECT ) );
		}
		return true;
	case CHC_CANMAKESKILL:
		{
			pszKey += 12;
			ITEMID_TYPE id = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, pszKey );
			sVal.FormatVal( Skill_MakeItem( id,	UID_CLEAR, SKTRIG_SELECT, true ) );
		}
		return true;
	case CHC_CANMOVE:
		{
			pszKey += 7;
			GETNONWHITESPACE(pszKey);
			CPointBase	ptDst	= GetTopPoint();
			DIR_TYPE	dir = GetDirStr(pszKey);
			ptDst.Move( dir );
			WORD		wBlockFlags	= 0;
			CRegionBase	*	pArea;
			if ( IsSetEF( EF_WalkCheck ) )
				pArea = CheckValidMove_New( ptDst, &wBlockFlags, dir, NULL );
			else
				pArea = CheckValidMove( ptDst, &wBlockFlags, dir );
			sVal.FormatHex( pArea ? pArea->GetResourceID() : 0 );
		}
		return true;
	case CHC_MOUNT:
		{
			CChar *pChar = Horse_GetMountChar();
			if ( pChar == NULL ) sVal.FormatVal(0);
			else sVal.FormatVal(pChar->GetUID());
			return true;
		}
	case CHC_MOVE:
		{
			pszKey += 4;
			GETNONWHITESPACE(pszKey);
			CPointBase	ptDst	= GetTopPoint();
			ptDst.Move( GetDirStr( pszKey ) );
			CRegionBase * pArea = ptDst.GetRegion( REGION_TYPE_MULTI | REGION_TYPE_AREA );
			if ( !pArea )
				sVal.FormatHex( -1 );
			else
			{
				WORD		wBlockFlags	= 0;
				signed char z;
				if ( IsSetEF( EF_WalkCheck ) )
					z = g_World.GetHeightPoint_New( ptDst, wBlockFlags, true );
				else
					z = g_World.GetHeightPoint( ptDst, wBlockFlags, true );
				sVal.FormatHex( wBlockFlags );
			}
		}
		return true;
	case CHC_DISPIDDEC:	// for properties dialog.
		sVal.FormatVal( pCharDef->m_trackID );
		return true;
	case CHC_GUILDABBREV:
		{
			LPCTSTR pszAbbrev = Guild_Abbrev(MEMORY_GUILD);
			sVal = ( pszAbbrev ) ? pszAbbrev : "";
		}
		return true;
	case CHC_ID:
		sVal = g_Cfg.ResourceGetName( pCharDef->GetResourceID());
		return true;
	case CHC_ISGM:
		sVal.FormatVal( IsPriv(PRIV_GM));
		return( true );
	case CHC_ISMYPET:
		sVal = NPC_IsOwnedBy( pCharSrc, true ) ? "1" : "0";
		return( true );
	case CHC_ISONLINE:
		if ( m_pPlayer != NULL )
		{
			sVal = IsClient() ? "1" : "0";
			return ( true );
		}
		if ( m_pNPC != NULL )
		{
			sVal = IsDisconnected() ? "0" : "1";
			return ( true );
		}
		sVal = 0;
		return( true );
	case CHC_ISPLAYER:
		sVal = (m_pPlayer != NULL) ? "1" : "0";
		return( true );
	case CHC_ISSTUCK:
		{
			CPointBase	pt = GetTopPoint();

			if ( LayerFind(LAYER_FLAG_Stuck) )					// it is stuck if is in web/etc
				sVal.FormatVal(0);
			else if ( CanMoveWalkTo(pt, true, true, DIR_N) ||	// it is stuck if can not move to N/E/S/W
				 CanMoveWalkTo(pt, true, true, DIR_E) ||
				 CanMoveWalkTo(pt, true, true, DIR_S) ||
				 CanMoveWalkTo(pt, true, true, DIR_W) )
				 sVal.FormatVal(1);
			else
				sVal.FormatVal(0);
			return true;
		}
	case CHC_ISTEVENT:
		if ( pszKey[8] != '.' )
			return( false );
		pszKey += 9;
		sVal = ( pCharDef->m_TEvents.FindResourceName(RES_EVENTS, pszKey) >= 0 ) ? "1" : "0";
		return true;
	case CHC_ISVENDOR:
		sVal = ( NPC_IsVendor()) ? "1" : "0";
		return( true );
	case CHC_MEMORY:
		// What is our memory flags about this pSrc person.
		{
			DWORD dwFlags	= 0;
			CItemMemory *	pMemory;
			pszKey	+= 6;
			if ( *pszKey == '.' )
			{
				pszKey++;
				CGrayUID		uid	= Exp_GetVal( pszKey );
				pMemory	= Memory_FindObj( uid );
			}			
			else
				pMemory	= Memory_FindObj( pCharSrc );
			if ( pMemory != NULL )
			{
				dwFlags = pMemory->GetMemoryTypes();
			}
			sVal.FormatHex( dwFlags );
		}
		return( true );
	case CHC_NAME:
		sVal = GetName( false );
		break;
	case CHC_SKILLTOTAL:
		{
			pszKey	+= 10;
			SKIP_SEPERATORS(pszKey);
			GETNONWHITESPACE(pszKey);
			int		iVal	= 0;
			bool	fComp	= true;
			if ( *pszKey == '\0' )
				;
			else if ( *pszKey == '+' )
				iVal	= Exp_GetVal( ++pszKey );
			else if ( *pszKey == '-' )
				iVal	= - Exp_GetVal( ++pszKey );
			else
			{
				iVal	= Exp_GetVal( pszKey );
				fComp	= false;
			}

			int iTotal = 0;
			int	iBase;
			for ( int i=0; i < g_Cfg.m_iMaxSkill; i++ )
			{
				iBase	= Skill_GetBase((SKILL_TYPE) i);
				if ( fComp )
				{
					if ( iVal < 0 )
					{
						if ( iBase >= -iVal )
							continue;
					}
					else if ( iBase < iVal )
						continue;
				}
				else
				{
					// check group flags
					CSkillDef *	pSkill	= g_Cfg.GetSkillDef( (SKILL_TYPE) i );
					if ( !pSkill )
						continue;
					if ( !( pSkill->m_dwGroup & iVal ) )
						continue;
				}
				iTotal += iBase;
			}
			sVal.FormatVal( iTotal );
		}
		return( true );
	case CHC_TOWNABBREV:
		{
			LPCTSTR pszAbbrev = Guild_Abbrev(MEMORY_TOWN);
			sVal = ( pszAbbrev ) ? pszAbbrev : "";
		}
		return true;
	case CHC_MAXWEIGHT:
		sVal.FormatVal( g_Cfg.Calc_MaxCarryWeight(this));
		return( true );
	case CHC_MODMAXWEIGHT:
		sVal.FormatVal( m_ModMaxWeight );
		return( true );
	case CHC_ACCOUNT:
		if ( pszKey[7] == '.' )	// used as a ref ?
		{
			if ( m_pPlayer != NULL )
			{
				pszKey += 7;
				SKIP_SEPERATORS(pszKey);
				CScriptObj * pRef = m_pPlayer->GetAccount();
				if ( pRef )
				{
					if ( pRef->r_WriteVal( pszKey, sVal, pSrc ) )
						break;
					return ( false );
				}
			}
		}
		if ( m_pPlayer == NULL )
			sVal.Empty();
		else
			sVal = m_pPlayer->GetAccount()->GetName();
		break;
	case CHC_ACT:
		if ( pszKey[3] == '.' )	// used as a ref ?
			goto do_default;
		sVal.FormatHex( m_Act_Targ.GetObjUID());	// uid
		break;
	case CHC_ACTDIFF:
		sVal.FormatVal( m_Act_Difficulty * 10 );
		break;
	case CHC_ACTARG1:
		sVal.FormatHex( m_atUnk.m_Arg1);
		break;
	case CHC_ACTARG2:
		sVal.FormatHex( m_atUnk.m_Arg2 );
		break;
	case CHC_ACTARG3:
		sVal.FormatHex( m_atUnk.m_Arg3 );
		break;
	case CHC_ACTION:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_SKILL, Skill_GetActive()));
		break;
	case CHC_BODY:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_CHARDEF, GetDispID()));
		break;
	case CHC_CREATE:
		sVal.FormatHex( -( g_World.GetTimeDiff( m_timeCreate ) / TICK_PER_SEC ));
		break;
	case CHC_DIR:
		sVal.FormatVal( m_dirFace );
		break;
	case CHC_EMOTEACT:
		sVal.FormatVal( IsStatFlag( STATF_EmoteAction ));
		break;
	case CHC_FLAGS:
		sVal.FormatHex( m_StatFlag );
		break;
	case CHC_FONT:
		sVal.FormatVal( m_fonttype );
		break;
	case CHC_FOOD:
		sVal.FormatVal( Stat_GetVal( STAT_FOOD ) );
		break;
	case CHC_HITPOINTS:
	case CHC_HITS:
		sVal.FormatVal( Stat_GetVal(STAT_STR) );
		break;
	case CHC_STAM:
	case CHC_STAMINA:
		sVal.FormatVal( Stat_GetVal(STAT_DEX) );
		break;
	case CHC_MANA:
		sVal.FormatVal( Stat_GetVal(STAT_INT) );
		break;
	case CHC_MAXHITS:
		sVal.FormatVal( Stat_GetMax(STAT_STR) );
		break;
	case CHC_MAXMANA:
		sVal.FormatVal( Stat_GetMax(STAT_INT) );
		break;
	case CHC_MAXSTAM:
		sVal.FormatVal( Stat_GetMax(STAT_DEX) );
		break;

	case CHC_HOME:
		sVal = m_ptHome.WriteUsed();
		break;
	case CHC_NIGHTSIGHT:
		sVal.FormatVal( IsStatFlag( STATF_NightSight ));
		break;
	case CHC_NOTOGETFLAG:
		{
			pszKey += 11;
			GETNONWHITESPACE(pszKey);
			CGrayUID uid = Exp_GetVal( pszKey );
			SKIP_ARGSEP( pszKey );
			bool fAllowIncog = ( Exp_GetVal( pszKey ) >= 1 );
			CChar * pChar;
			
			if ( ! uid.IsValidUID() )
				pChar = pCharSrc;
			else
			{
				pChar = uid.CharFind();
				if ( ! pChar )
					pChar = pCharSrc;
			}
			sVal.FormatVal( Noto_GetFlag( pChar, fAllowIncog ) );
		}
		break;
	case CHC_NPC:
		goto do_default;

	case CHC_OBODY:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_CHARDEF, m_prev_id ));
		break;

	case CHC_OSKIN:
		sVal.FormatHex( m_prev_Hue );
		break;
	case CHC_P:
		goto do_default;
	case CHC_STONE:
		sVal.FormatVal( IsStatFlag( STATF_Stone ));
		break;
	case CHC_TITLE:
		sVal = m_sTitle;
		break;
	case CHC_LIGHT:
		sVal.FormatHex(m_LocalLight);
		break;
	case CHC_EXP:
		sVal.FormatVal(m_exp);
		break;
	case CHC_LEVEL:
		sVal.FormatVal(m_level);
		break;
	case CHC_VISUALRANGE:
		sVal.FormatVal(GetSight());
	default:
		DEBUG_CHECK(0);
		return( false );
	}
	return true;
	EXC_CATCH("CChar");
	return false;
}



bool CChar::r_LoadVal( CScript & s )
{
	LOCKDATA;
	EXC_TRY(("r_LoadVal('%s %s')", s.GetKey(), s.GetArgStr()));
	CHC_TYPE iKeyNum = (CHC_TYPE) FindTableHeadSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 );
	if ( iKeyNum < 0 )
	{
do_default:
		if ( m_pPlayer )
		{
			if ( m_pPlayer->r_LoadVal( this, s ))
				return( true );
		}
		if ( m_pNPC )
		{
			if ( m_pNPC->r_LoadVal( this, s ))
				return( true );
		}

		{
			LPCTSTR		pszKey	=  s.GetKey();
			int i = g_Cfg.FindSkillKey( pszKey );
			if ( i != SKILL_NONE )
			{
				// Check some skill name.
				Skill_SetBase( (SKILL_TYPE) i, s.GetArgVal() );
				return true;
			}

			i = g_Cfg.FindStatKey( pszKey );
			if ( i >= 0 )
			{
				// if ( g_Serv.IsLoading() || (m_Stat[i].m_base == 0) )
				//	m_Stat[i].m_base	= s.GetArgVal();
				// else
				Stat_SetBase( (STAT_TYPE) i, s.GetArgVal() - Stat_GetMod( (STAT_TYPE) i ) );
						// - Stat_GetAdjusted((STAT_TYPE)i)
						
				return true;
			}

			if ( !strnicmp( pszKey, "O", 1 ) )
			{
				i = g_Cfg.FindStatKey( pszKey+1 );
				if ( i >= 0 )
				{
					Stat_SetBase( (STAT_TYPE) i, s.GetArgVal() );
					return true;
				}
			}
			if ( !strnicmp( pszKey, "MOD", 3 ) )
			{
				i = g_Cfg.FindStatKey( pszKey+3 );
				if ( i >= 0 )
				{
					Stat_SetMod( (STAT_TYPE) i, s.GetArgVal() );
					return true;
				}
			}
		}

		return( CObjBase::r_LoadVal( s ));
	}

	switch (iKeyNum)
	{
	case CHC_MAXHITS:
		Stat_SetMax(STAT_STR, s.HasArgs() ? (s.GetArgVal() - Stat_GetMod(STAT_STR)) : 0 );
		break;
	case CHC_MAXMANA:
		Stat_SetMax(STAT_INT, s.HasArgs() ? (s.GetArgVal() - Stat_GetMod(STAT_INT)) : 0 );
		break;
	case CHC_MAXSTAM:
		Stat_SetMax(STAT_DEX, s.HasArgs() ? (s.GetArgVal() - Stat_GetMod(STAT_DEX)) : 0 );
		break;
	case CHC_ACCOUNT:
		return SetPlayerAccount( s.GetArgStr());
	case CHC_ACT:
		m_Act_Targ = s.GetArgVal();
		break;
	case CHC_ACTDIFF:
		m_Act_Difficulty = (s.GetArgVal() / 10);
		break;
	case CHC_ACTARG1:
		m_atUnk.m_Arg1 = s.GetArgVal();
		break;
	case CHC_ACTARG2:
		m_atUnk.m_Arg2 = s.GetArgVal();
		break;
	case CHC_ACTARG3:
		m_atUnk.m_Arg3 = s.GetArgVal();
		break;
	case CHC_ACTION:
		return Skill_Start( g_Cfg.FindSkillKey( s.GetArgStr()));
	case CHC_BODY:
		SetID( (CREID_TYPE) g_Cfg.ResourceGetIndexType( RES_CHARDEF, s.GetArgStr()));
		break;
	case CHC_CREATE:
		m_timeCreate = CServTime::GetCurrentTime() - ( s.GetArgVal() * TICK_PER_SEC );
		break;
	case CHC_DIR:
		m_dirFace = (DIR_TYPE) s.GetArgVal();
		if ( m_dirFace < 0 || m_dirFace >= DIR_QTY )
			m_dirFace = DIR_SE;
		break;
	case CHC_DISMOUNT:
		Horse_UnMount();
		break;
	case CHC_EMOTEACT:
		{
			bool fSet = IsStatFlag(STATF_EmoteAction);
			if ( s.HasArgs())
			{
				fSet = s.GetArgVal() ? true : false;
			}
			else
			{
				fSet = ! fSet;
			}
			StatFlag_Mod(STATF_EmoteAction,fSet);
		}
		break;
	case CHC_FLAGS:		// DO NOT MODIFY STATF_SaveParity, STATF_Spawned, STATF_Pet
		m_StatFlag = ( s.GetArgVal() &~ (STATF_SaveParity|STATF_Pet|STATF_Spawned)) | ( m_StatFlag & (STATF_SaveParity|STATF_Pet|STATF_Spawned) );
		break;
	case CHC_FONT:
		m_fonttype = (FONT_TYPE) s.GetArgVal();
		if ( m_fonttype < 0 || m_fonttype >= FONT_QTY )
			m_fonttype = FONT_NORMAL;
		break;
	case CHC_FOOD:
		Stat_SetVal(STAT_FOOD, s.GetArgVal());
		break;
	case CHC_HITPOINTS:
	case CHC_HITS:
		Stat_SetVal(STAT_STR,  s.GetArgVal() );
		UpdateHitsFlag();
		break;
	case CHC_MANA:
		Stat_SetVal(STAT_INT,  s.GetArgVal() );
		UpdateManaFlag();
		break;
	case CHC_MODMAXWEIGHT:
		m_ModMaxWeight = s.GetArgVal();
		break;
	case CHC_STAM:
	case CHC_STAMINA:
		Stat_SetVal(STAT_DEX,  s.GetArgVal() );
		UpdateStamFlag();
		break;
	case CHC_HOME:
		if ( ! s.HasArgs())
			m_ptHome = GetTopPoint();
		else
			m_ptHome.Read( s.GetArgStr());
		break;
	case CHC_NAME:
	case CHC_FAME:
	case CHC_KARMA:
		goto do_default;
	case CHC_MEMORY:
		{
			int piCmd[2];
			int iArgQty = Str_ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd) );
			if ( iArgQty < 2 )
				return( false );

			CGrayUID	uid		= piCmd[0];
			DWORD		dwFlags	= piCmd[1];

			CItemMemory * pMemory = Memory_FindObj( uid );
			if ( pMemory != NULL )
				pMemory->SetMemoryTypes( piCmd[1] );
			else
				pMemory = Memory_AddObjTypes( uid, piCmd[1] );
		}
		break;
	case CHC_NIGHTSIGHT:
		{
			bool fNightsight;
			if ( s.HasArgs())
			{
				fNightsight = s.GetArgVal();
			}
			else
			{
				fNightsight = ! IsStatFlag(STATF_NightSight);
			}
			StatFlag_Mod( STATF_NightSight, fNightsight );
			Update();
		}
		break;
	case CHC_NPC:
		return SetNPCBrain( (NPCBRAIN_TYPE) s.GetArgVal());
	case CHC_OBODY:
		{
			CREID_TYPE id = (CREID_TYPE) g_Cfg.ResourceGetIndexType( RES_CHARDEF, s.GetArgStr());
			if ( ! CCharBase::FindCharBase( id ))
			{
				DEBUG_ERR(( "OBODY Invalid Char 0%x" DEBUG_CR, id ));
				return( false );
			}
			m_prev_id = id;
		}
		break;
	case CHC_OSKIN:
		m_prev_Hue = s.GetArgVal();
		break;
	case CHC_P:
		{
			CPointMap pt;
			pt.Read(s.GetArgStr());
			m_fClimbUpdated = false; // update climb height
			MoveToChar(pt);
		}
		break;
	case CHC_STONE:
		{
			bool fSet;
			bool fChange = IsStatFlag(STATF_Stone);
			if ( s.HasArgs())
			{
				fSet = s.GetArgVal() ? true : false;
				fChange = ( fSet != fChange );
			}
			else
			{
				fSet = ! fChange;
				fChange = true;
			}
			StatFlag_Mod(STATF_Stone,fSet);
			if ( fChange )
			{
				RemoveFromView();
				Update();
			}
		}
		break;
	case CHC_TITLE:
		m_sTitle = s.GetArgStr();
		break;
	case CHC_LIGHT:
		m_LocalLight = s.GetArgVal();
		break;
	case CHC_EXP:
		m_exp = s.GetArgVal();
		ChangeExperience();			//	auto-update level if applicable
		break;
	case CHC_LEVEL:
		m_level = s.GetArgVal();
		break;
	case CHC_VISUALRANGE:
		{
			BYTE bIn = s.GetArgVal();
			if ( bIn <= UO_MAP_VIEW_RADAR )
				return( false );

			SetSight(bIn);
		}
		break;
	default:
		DEBUG_CHECK(0);
		return( false );
	}
	return true;
	EXC_CATCH("CChar");
	return false;
}

void CChar::r_Write( CScript & s )
{
	if ( IsSetEF(EF_Size_Optimise) ) s.WriteSection("WC %s", GetResourceName());
	else s.WriteSection("WORLDCHAR %s", GetResourceName());
	s.WriteKeyHex( "CREATE", -( g_World.GetTimeDiff( m_timeCreate ) / TICK_PER_SEC ) );

	CObjBase::r_Write( s );
	if ( m_pPlayer )
	{
		m_pPlayer->r_WriteChar(this, s);
	}
	if ( m_pNPC )
	{
		m_pNPC->r_WriteChar(this, s);
	}

	if ( GetTopPoint().IsValidPoint())
	{
		s.WriteKey( "P", GetTopPoint().WriteUsed());
	}
	if ( ! m_sTitle.IsEmpty())
	{
		s.WriteKey( "TITLE", m_sTitle );
	}
	if ( m_fonttype != FONT_NORMAL )
	{
		s.WriteKeyVal( "FONT", m_fonttype );
	}
	if ( m_dirFace != DIR_SE )
	{
		s.WriteKeyVal( "DIR", m_dirFace );
	}
	if ( m_prev_id != GetID())
	{
		s.WriteKey( "OBODY", g_Cfg.ResourceGetName( RESOURCE_ID( RES_CHARDEF, m_prev_id )));
	}
	if ( m_prev_Hue != HUE_DEFAULT )
	{
		s.WriteKeyHex( "OSKIN", m_prev_Hue );
	}
	if ( m_StatFlag )
	{
		s.WriteKeyHex( "FLAGS", m_StatFlag );
	}
	if ( m_LocalLight )
	{
		s.WriteKeyHex( "LIGHT", m_LocalLight );
	}
	if ( Skill_GetActive() != SKILL_NONE )
	{
		s.WriteKey( "ACTION", g_Cfg.ResourceGetName( RESOURCE_ID( RES_SKILL, Skill_GetActive())));
		if ( m_atUnk.m_Arg1 )
		{
			s.WriteKeyHex( "ACTARG1", m_atUnk.m_Arg1 );
		}
		if ( m_atUnk.m_Arg2 )
		{
			s.WriteKeyHex( "ACTARG2", m_atUnk.m_Arg2 );
		}
		if ( m_atUnk.m_Arg3 )
		{
			s.WriteKeyHex( "ACTARG3", m_atUnk.m_Arg3 );
		}
	}

	if ( m_ModMaxWeight )
		s.WriteKeyVal( "MODMAXWEIGHT", m_ModMaxWeight );

	if ( m_exp )
		s.WriteKeyVal("EXP", m_exp);
	if ( m_level )
		s.WriteKeyVal("LEVEL", m_level);

	s.WriteKeyVal( "HITS", Stat_GetVal(STAT_STR) );
	s.WriteKeyVal( "STAM", Stat_GetVal(STAT_DEX) );
	s.WriteKeyVal( "MANA", Stat_GetVal(STAT_INT) );
	s.WriteKeyVal( "FOOD", Stat_GetVal(STAT_FOOD) );

	if ( Stat_GetAdjusted(STAT_STR) != Stat_GetMax(STAT_STR) )
		s.WriteKeyVal( "MAXHITS", Stat_GetMax(STAT_STR) );
	if ( Stat_GetAdjusted(STAT_DEX) != Stat_GetMax(STAT_DEX) )
		s.WriteKeyVal( "MAXSTAM", Stat_GetMax(STAT_DEX) );
	if ( Stat_GetAdjusted(STAT_INT) != Stat_GetMax(STAT_INT) )
		s.WriteKeyVal( "MAXMANA", Stat_GetMax(STAT_INT) );

	if ( m_ptHome.IsValidPoint())
	{
		s.WriteKey( "HOME", m_ptHome.WriteUsed());
	}

	TCHAR	szTmp[100];
	int j=0;
	for ( j=0;j<STAT_QTY;j++)
	{
		// this is VERY important, saving the MOD first
		if ( Stat_GetMod( (STAT_TYPE) j ) )
		{
			sprintf( szTmp, "MOD%s",  g_Stat_Name[j] );
			s.WriteKeyVal( szTmp, Stat_GetMod( (STAT_TYPE) j ) );
		}
		if ( Stat_GetBase( (STAT_TYPE) j ) )
		{
			sprintf( szTmp, "O%s",  g_Stat_Name[j] );
			s.WriteKeyVal( szTmp, Stat_GetBase( (STAT_TYPE) j ) );
		}
	}


	for ( j=0;j<MAX_SKILL;j++)
	{
		if ( ! m_Skill[j] )
			continue;
		s.WriteKeyVal( g_Cfg.GetSkillDef( (SKILL_TYPE) j )->GetKey(), Skill_GetBase( (SKILL_TYPE) j ));
	}

	r_WriteContent(s);
}

void CChar::r_WriteParity( CScript & s )
{
	// overload virtual for world save.

	// if ( GetPrivLevel() <= PLEVEL_Guest ) return;

	if ( g_World.m_fSaveParity == IsStatFlag(STATF_SaveParity))
	{
		return; // already saved.
	}

	StatFlag_Mod( STATF_SaveParity, g_World.m_fSaveParity );

	int iRet = IsWeird();
	if ( iRet )
	{
		DEBUG_MSG(( "Weird char %x,'%s' rejected from save %d." DEBUG_CR, (DWORD) GetUID(), (LPCTSTR) GetName(), iRet ));
		return;
	}

	r_WriteSafe(s);
}

bool CChar::r_Load( CScript & s ) // Load a character from script
{
	CScriptObj::r_Load(s);

	// Init the STATF_SaveParity flag.
	// StatFlag_Mod( STATF_SaveParity, g_World.m_fSaveParity );

	// Make sure everything is ok.
	if (( m_pPlayer && ! IsClient()) ||
		( m_pNPC && IsStatFlag( STATF_DEAD | STATF_Ridden )))	// dead npc
	{
		DEBUG_CHECK( ! IsClient());
		SetDisconnected();
	}
	int iResultCode = CObjBase::IsWeird();
	if ( iResultCode )
	{
		DEBUG_ERR(( "Char 0%x Invalid, id='%s', code=0%x" DEBUG_CR, (DWORD) GetUID(), (LPCTSTR) GetResourceName(), iResultCode ));
		Delete();
	}

	return( true );
}

enum CHV_TYPE
{
	CHV_AFK,
	CHV_ALLSKILLS,
	CHV_ANIM,		// "Bow, "Salute"
	CHV_ATTACK,
	CHV_BANK,
	CHV_BARK,
	CHV_BOUNCE,
	CHV_BOW,
	CHV_CONSUME,
	CHV_CONTROL,
	CHV_CRIMINAL,
	CHV_DISCONNECT,
	CHV_DRAWMAP,
	CHV_DROP,
	CHV_DUPE,
	CHV_EQUIP,	// engage the equip triggers.
	CHV_EQUIPARMOR,
	CHV_EQUIPHALO,
	CHV_EQUIPWEAPON,
	CHV_FACE,
	CHV_FORGIVE,
	CHV_GO,
	CHV_GOCHAR,
	CHV_GOCHARID,
	CHV_GOCLI,
	CHV_GOITEMID,
	CHV_GONAME,
	CHV_GOSOCK,
	CHV_GOTYPE,
	CHV_GOUID,
	CHV_HEAR,
	CHV_HUNGRY,
	CHV_INVIS,
	CHV_INVUL,
	CHV_JAIL,
	CHV_KILL,
	CHV_MAKEITEM,
	CHV_NEWBIESKILL,
	CHV_NEWDUPE,
	CHV_NEWITEM,
	CHV_NEWNPC,
	CHV_PACK,
	CHV_POISON,
	CHV_POLY,
	CHV_PRIVSET,
	CHV_RELEASE,
	CHV_REMOVE,
	CHV_RESURRECT,
	CHV_SALUTE,	// salute to player
	CHV_SKILL,
	CHC_SKILLGAIN,
	CHV_SLEEP,
	CHV_SMSG,
	CHV_SMSGU,
	CHV_SUICIDE,
	CHV_SUMMONCAGE,
	CHV_SUMMONTO,
	CHV_SYSMESSAGE,
	CHV_SYSMESSAGEUA,
	CHV_UNDERWEAR,
	CHV_UNEQUIP,	// engage the unequip triggers.
	CHV_WHERE,
	CHV_QTY,
};

LPCTSTR const CChar::sm_szVerbKeys[CHV_QTY+1] =
{
	"AFK",
	"ALLSKILLS",
	"ANIM",		// "Bow", "Salute"
	"ATTACK",
	"BANK",
	"BARK",
	"BOUNCE",
	"BOW",
	"CONSUME",
	"CONTROL",
	"CRIMINAL",
	"DISCONNECT",
	"DRAWMAP",
	"DROP",
	"DUPE",
	"EQUIP",	// engage the equip triggers.
	"EQUIPARMOR",
	"EQUIPHALO",
	"EQUIPWEAPON",
	"FACE",
	"FORGIVE",
	"GO",
	"GOCHAR",
	"GOCHARID",
	"GOCLI",
	"GOITEMID",
	"GONAME",
	"GOSOCK",
	"GOTYPE",
	"GOUID",
	"HEAR",
	"HUNGRY",
	"INVIS",
	"INVUL",
	"JAIL",
	"KILL",
	"MAKEITEM",
	"NEWBIESKILL",
	"NEWDUPE",
	"NEWITEM",
	"NEWNPC",
	"PACK",
	"POISON",
	"POLY",
	"PRIVSET",
	"RELEASE",
	"REMOVE",
	"RESURRECT",
	"SALUTE",	//	salute to player
	"SKILL",
	"SKILLGAIN",
	"SLEEP",
	"SMSG",
	"SMSGU",
	"SUICIDE",
	"SUMMONCAGE",
	"SUMMONTO",
	"SYSMESSAGE",
	"SYSMESSAGEUA",
	"UNDERWEAR",
	"UNEQUIP",	// engage the unequip triggers.
	"WHERE",
	NULL,
};

bool CChar::r_Verb( CScript &s, CTextConsole * pSrc ) // Execute command from script
{
	LOCKDATA;
	EXC_TRY(("r_Verb('%s %s',%x)", s.GetKey(), s.GetArgStr(), pSrc));
	if ( pSrc == NULL )
	{
		return( false );
	}
	if ( IsClient())	// I am a client so i get an expanded verb set. Things only clients can do.
	{
		if ( GetClient()->r_Verb( s, pSrc ))
			return( true );
	}


	int index = FindTableSorted( s.GetKey(), sm_szVerbKeys, COUNTOF(sm_szVerbKeys)-1 );
	bool	fNewAct	= false;
	switch( index )
	{
		case CHV_NEWDUPE:	// uid
		case CHV_NEWITEM:	// just add an item but don't put it anyplace yet..
		case CHV_NEWNPC:
			fNewAct	= true;
			index	= -1;
			break;
	}

	if ( index < 0 )
	{
		if ( m_pNPC )
		{
			if ( NPC_OnVerb( s, pSrc ))
				return( true );
		}
		if ( m_pPlayer )
		{
			if ( Player_OnVerb( s, pSrc ))
				return( true );
		}
		bool	fRet	= ( CObjBase::r_Verb( s, pSrc ));
		if ( fNewAct )
			m_Act_Targ = g_World.m_uidNew;
		return fRet;
	}

	CChar * pCharSrc = pSrc->GetChar();

	switch ( index )
	{
	case CHV_AFK:
		// toggle ?
		{
			bool fAFK = ( Skill_GetActive() == NPCACT_Napping );
			bool fMode;
			if ( s.HasArgs())
			{
				fMode = s.GetArgVal();
			}
			else
			{
				fMode = ! fAFK;
			}
			if ( fMode != fAFK )
			{
				if ( fMode )
				{
					SysMessageDefault(DEFMSG_CMDAFK_ENTER);
					m_Act_p = GetTopPoint();
					Skill_Start( NPCACT_Napping );
				}
				else
				{
					SysMessageDefault(DEFMSG_CMDAFK_LEAVE);
					Skill_Start( SKILL_NONE );
				}
			}
		}
		break;

	case CHV_ALLSKILLS:
		{
			int iVal = s.GetArgVal();
			for ( int i=0; i<MAX_SKILL; i++ )
			{
				Skill_SetBase( (SKILL_TYPE)i, iVal );
			}
		}
		break;
	case CHV_ANIM:
		// ANIM, ANIM_TYPE action, bool fBackward = false, BYTE iFrameDelay = 1
		{
			int Arg_piCmd[3];		// Maximum parameters in one line
			int Arg_Qty = Str_ParseCmds( s.GetArgRaw(), Arg_piCmd, COUNTOF(Arg_piCmd));

			return UpdateAnimate( (ANIM_TYPE) Arg_piCmd[0], false,
				( Arg_Qty > 1 )	? Arg_piCmd[1] : false,
				( Arg_Qty > 2 )	? Arg_piCmd[2] : 1 );
		}
		break;
	case CHV_ATTACK:
		{
			CChar *		pSrc	= pCharSrc;
			int piCmd[1];
			if ( Str_ParseCmds( s.GetArgRaw(), piCmd, COUNTOF(piCmd)) )
			{
				CGrayUID	uid	= piCmd[0];
				pSrc	= uid.CharFind();
			}
			if ( pSrc )
				Fight_Attack( pSrc );
		}
		break;
	case CHV_BANK:
		// Open the bank box for this person
		if ( pCharSrc == NULL || ! pCharSrc->IsClient() )
			return( false );
		pCharSrc->GetClient()->addBankOpen( this, (LAYER_TYPE)((s.HasArgs()) ? s.GetArgVal() : LAYER_BANKBOX ));
		break;
	case CHV_BARK:
		SoundChar( (CRESND_TYPE) ( s.HasArgs() ? s.GetArgVal() : ( Calc_GetRandVal(2) ? CRESND_RAND1 : CRESND_RAND2 )));
		break;
	case CHV_BOUNCE: // uid
		return ItemBounce( CGrayUID( s.GetArgVal()).ItemFind());
	case CHV_BOW:
		UpdateDir( pCharSrc );
		UpdateAnimate( ANIM_BOW, false );
		break;

	case CHV_CONTROL: // Possess
		if ( pCharSrc == NULL || ! pCharSrc->IsClient())
			return( false );
		return( pCharSrc->GetClient()->Cmd_Control( this ));

	case CHV_CONSUME:
		{
		CResourceQtyArray Resources;
		Resources.Load( s.GetArgStr() );
		ResourceConsume( &Resources, 1, false );
		}
		break;
	case CHV_CRIMINAL:
		if ( s.HasArgs() && ! s.GetArgVal())
		{
			StatFlag_Clear( STATF_Criminal );
		}
		else
		{
			Noto_Criminal();
		}
		break;
	case CHV_DISCONNECT:
		// Push a player char off line. CLIENTLINGER thing
		if ( IsClient())
		{
			return GetClient()->addKick( pSrc, false );
		}
		SetDisconnected();
		break;
	case CHV_DRAWMAP:
		// Use the cartography skill to draw a map.
		// Already did the skill check.
		m_atCartography.m_Dist = s.GetArgVal();
		Skill_Start( SKILL_CARTOGRAPHY );
		break;
	case CHV_DROP:	// uid
		return ItemDrop( CGrayUID( s.GetArgVal()).ItemFind(), GetTopPoint());
	case CHV_DUPE:	// = dupe a creature !
		CChar::CreateNPC( GetID())->MoveNearObj( this, 1 );
		break;
	case CHV_EQUIP:	// uid
		return ItemEquip( CGrayUID( s.GetArgVal()).ItemFind());
	case CHV_EQUIPHALO:
		{
			// equip a halo light
			CItem * pItem = CItem::CreateScript(ITEMID_LIGHT_SRC, this);
			ASSERT( pItem);
			if ( s.HasArgs())	// how long to last ?
			{
				int iTimer = s.GetArgVal();
				if ( iTimer > 0 )
					pItem->SetTimeout(iTimer);
					
				pItem->Item_GetDef()->m_ttNormal.m_tData4 = 0;
			}
			pItem->SetAttr(ATTR_MOVE_NEVER);
			LayerAdd( pItem, LAYER_LIGHT );
		}
		return( true );
	case CHV_EQUIPARMOR:
		return ItemEquipArmor(false);
	case CHV_EQUIPWEAPON:
		// find my best waepon for my skill and equip it.
		return ItemEquipWeapon(false);
	case CHV_FACE:
		{
			CObjBase	*pTarget = pCharSrc;
			int		piCmd[1];
			if ( Str_ParseCmds(s.GetArgRaw(), piCmd, COUNTOF(piCmd)) )
			{
				CGrayUID uid = piCmd[0];
				pTarget = uid.ObjFind();
			}
			UpdateDir(pTarget);
			break;
		}
	case CHV_FORGIVE:
		Jail( pSrc, false, 0 );
		break;
	case CHV_GOCHAR:	// uid
		return TeleportToObj( 1, s.GetArgStr());
	case CHV_GOCHARID:
		return TeleportToObj( 3, s.GetArgStr());
	case CHV_GOCLI:	// enum clients
		return TeleportToCli( 1, s.GetArgVal());
	case CHV_GOITEMID:
		return TeleportToObj( 4, s.GetArgStr());
	case CHV_GONAME:
		return TeleportToObj( 0, s.GetArgStr());
	case CHV_GO:
		Spell_Teleport( g_Cfg.GetRegionPoint( s.GetArgStr()), true, false );
		break;
	case CHV_GOSOCK:	// sockid
		return TeleportToCli( 0, s.GetArgVal());
	case CHV_GOTYPE:
		return TeleportToObj( 2, s.GetArgStr());
	case CHV_GOUID:	// uid
		if ( s.HasArgs())
		{
			CGrayUID uid( s.GetArgVal());
			CObjBaseTemplate * pObj = uid.ObjFind();
			if ( pObj == NULL )
				return( false );
			pObj = pObj->GetTopLevelObj();
			Spell_Teleport( pObj->GetTopPoint(), true, false );
			return( true );
		}
		return( false );
	case CHV_HEAR:
		// NPC will hear this command but no-one else.
		if ( m_pPlayer )
		{
			SysMessage( s.GetArgStr());
		}
		else
		{
			ASSERT( m_pNPC );
			NPC_OnHear( s.GetArgStr(), pSrc->GetChar());
		}
		break;
	case CHV_HUNGRY:	// How hungry are we ?
		if ( pCharSrc )
		{
			char *z = Str_GetTemp();
			sprintf(z, "%s %s %s",
				( pCharSrc == this ) ? "You" : (LPCTSTR) GetName(),
				( pCharSrc == this ) ? "are" : "looks",
				Food_GetLevelMessage( false, false ));
			pCharSrc->ObjMessage(z, this);
		}
		break;
	case CHV_INVIS:
		if ( pSrc )
		{
			m_StatFlag = s.GetArgFlag( m_StatFlag, STATF_Insubstantial );
			pSrc->SysMessagef( "Invis %s", ( IsStatFlag( STATF_Insubstantial )) ? "ON" : "OFF" );
			UpdateMode( NULL, true );	// invis used by GM bug requires this
		}
		break;
	case CHV_INVUL:
		if ( pSrc )
		{
			m_StatFlag = s.GetArgFlag( m_StatFlag, STATF_INVUL );
			pSrc->SysMessagef( "Invulnerability %s", ( IsStatFlag( STATF_INVUL )) ? "ON" : "OFF" );
		}
		break;
	case CHV_JAIL:	// i am being jailed
		Jail( pSrc, true, s.GetArgVal() );
		break;
	case CHV_KILL:
		{
			Effect( EFFECT_LIGHTNING, ITEMID_NOTHING, pCharSrc );
			OnTakeDamage( 10000, pCharSrc, DAMAGE_GOD );
			Stat_SetVal( STAT_STR, 0 );
			g_Log.Event( LOGL_EVENT|LOGM_KILLS|LOGM_GM_CMDS, "'%s' was KILLed by '%s'" DEBUG_CR, (LPCTSTR) GetName(), (LPCTSTR) pSrc->GetName());
		}
		break;

	case CHV_MAKEITEM:
		return Skill_MakeItem(
			(ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgRaw()),
			m_Act_Targ, SKTRIG_START );

	case CHV_NEWBIESKILL:
		{
		CResourceLock s;
		if ( ! g_Cfg.ResourceLock( s, RES_NEWBIE, s.GetArgStr()))
			return( false );
		ReadScript( s );
		}
		break;
	case CHV_PACK:
		if ( pCharSrc == NULL || ! pCharSrc->IsClient())
			return( false );
		pCharSrc->m_pClient->addBankOpen( this, LAYER_PACK ); // Send Backpack (with items)
		break;

	case CHV_POISON:
		{
			int iSkill = s.GetArgVal();
			SetPoison( iSkill, iSkill/50, pSrc->GetChar());
		}
		break;

	case CHV_POLY:	// result of poly spell script choice. (casting a spell)

		m_atMagery.m_Spell = SPELL_Polymorph;
		m_atMagery.m_SummonID = (CREID_TYPE) g_Cfg.ResourceGetIndexType( RES_CHARDEF, s.GetArgStr());

		if ( m_pClient != NULL )
		{
			m_Act_Targ = m_pClient->m_Targ_UID;
			m_Act_TargPrv = m_pClient->m_Targ_PrvUID;
		}
		else
		{
			m_Act_Targ = GetUID();
			m_Act_TargPrv = GetUID();
		}
		Skill_Start( SKILL_MAGERY );
		break;

	case CHV_PRIVSET:
		return( SetPrivLevel( pSrc, s.GetArgStr()));
	case CHV_RELEASE:
		Skill_Start( SKILL_NONE );
		NPC_PetClearOwners();
		SoundChar( CRESND_RAND2 );	// No noise
		return( true );
	case CHV_REMOVE:	// remove this char from the world instantly.
		if ( m_pPlayer )
		{
			if ( s.GetArgRaw()[0] != '1' || pSrc->GetPrivLevel() < PLEVEL_Admin )
			{
				pSrc->SysMessage( "Can't remove players this way, Try 'kill','kick' or 'remove 1'" );
				return( false );
			}
			if ( GetClient() )
				GetClient()->addObjectRemove(this);
		}
		Delete();
		break;
	case CHV_RESURRECT:
		return OnSpellEffect( SPELL_Resurrection, pCharSrc, 1000, NULL );
	case CHV_SALUTE:	//	salute to player
		{
			CObjBase	*pTarget = pCharSrc;
			int		piCmd[1];
			if ( Str_ParseCmds(s.GetArgRaw(), piCmd, COUNTOF(piCmd)) )
			{
				CGrayUID uid = piCmd[0];
				pTarget = uid.ObjFind();
			}
			UpdateDir(pTarget);
			UpdateAnimate( ANIM_SALUTE, false );
			break;
		}
	case CHV_SKILL:
		Skill_Start( g_Cfg.FindSkillKey( s.GetArgStr()));
		break;
	case CHC_SKILLGAIN:
		{
			if ( s.HasArgs() )
			{
				TCHAR * ppArgs[2];
				if ( Str_ParseCmds( s.GetArgRaw(), ppArgs, COUNTOF( ppArgs )) )
				{
					SKILL_TYPE iSkill = g_Cfg.FindSkillKey( ppArgs[0] );
					if ( iSkill == SKILL_NONE )
						return( false );
					Skill_Experience( iSkill, Exp_GetVal( ppArgs[1] ));
				}
			}
		}
		return( true );
	case CHV_SLEEP:
		SleepStart( s.GetArgVal());
		break;
	case CHV_SUICIDE:
		Memory_ClearTypes( MEMORY_FIGHT ); // Clear the list of people who get credit for your death
		UpdateAnimate( ANIM_SALUTE );
		Stat_SetVal( STAT_STR, 0 );
		break;
	case CHV_SUMMONCAGE: // i just got summoned
		if ( pCharSrc != NULL )
		{
			// Let's make a cage to put the player in
			ITEMID_TYPE id = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, "i_multi_cage" );
			if ( id < 0 )
				return( false );
			CItemMulti * pItem = dynamic_cast <CItemMulti*>( CItem::CreateBase( id ));
			if ( pItem == NULL )
				return( false );
			CPointMap pt = pCharSrc->GetTopPoint();
			pt.MoveN( pCharSrc->m_dirFace, 3 );
			pItem->MoveToDecay( pt, 10*60*TICK_PER_SEC );	// make the cage vanish after 10 minutes.
			pItem->Multi_Create( NULL, UID_CLEAR );
			Spell_Teleport( pt, true, false );
			break;
		}
		return( false );
	case CHV_SUMMONTO:	// i just got summoned
		if ( pCharSrc != NULL )
		{
			Spell_Teleport( pCharSrc->GetTopPoint(), true, false );
		}
		break;
	case CHV_SMSG:
	case CHV_SMSGU:
	case CHV_SYSMESSAGE:
	case CHV_SYSMESSAGEUA:
		// just eat this if it's not a client.
		break;
	case CHV_UNDERWEAR:
		if ( ! IsHuman())
			return( false );
		SetHue( GetHue() ^ HUE_UNDERWEAR );
		RemoveFromView();
		Update();
		break;
	case CHV_UNEQUIP:	// uid
		return ItemBounce( CGrayUID( s.GetArgVal()).ItemFind());
	case CHV_WHERE:
		if ( pCharSrc )
		{
			char *z = Str_GetTemp();
			if ( m_pArea )
			{
				if ( m_pArea->GetResourceID().IsItem())
				{
basicform:
					sprintf(z, "I am in %s (%s)", m_pArea->GetName(), GetTopPoint().WriteUsed());
				}
				else
				{
					const CRegionBase * pRoom = GetTopPoint().GetRegion( REGION_TYPE_ROOM );
					if ( ! pRoom ) 
						goto basicform;
					sprintf(z, "I am in %s in %s (%s)", m_pArea->GetName(), pRoom->GetName(), GetTopPoint().WriteUsed());
				}
			}
			else sprintf(z, "I am at %s.", GetTopPoint().WriteUsed());
			pCharSrc->ObjMessage(z, this);
		}
		break;
	default:
		DEBUG_CHECK(0);
		return( false );
	}
	return true;
	EXC_CATCH("CChar");
	return false;
}

CItemBase *		CChar::GetKeyItemBase( LPCTSTR pszKey )
{
	const char	*		sID	= GetKeyStr( pszKey );
	if ( sID && *sID )
	{
		ITEMID_TYPE id = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, sID );
		return CItemBase::FindItemBase( id );
	}
	return NULL;
}


bool	CChar::OnTriggerSpeech( LPCTSTR pszName, LPCTSTR pszText, CChar * pSrc, TALKMODE_TYPE & mode )
{
	
	CScriptObj *	pDef	= g_Cfg.ResourceGetDefByName( RES_SPEECH, pszName );
	if ( !pDef )
	{
		DEBUG_ERR( ("OnTriggerSpeech: couldn't find speech resource %s" DEBUG_CR, pszName ) );
		return false;
	}

	CResourceLink * pLink	= dynamic_cast <CResourceLink *>( pDef );
	if ( !pLink )
	{
		DEBUG_ERR( ("OnTriggerSpeech: couldn't find speech %s" DEBUG_CR, pszName ) );
		return false;
	}

	CResourceLock	s;
	if ( !pLink->ResourceLock(s) )
		return false;

	if ( ! pLink->HasTrigger(XTRIG_UNKNOWN))
		return false;

	if ( OnHearTrigger( s, pszText, pSrc, mode ) == TRIGRET_RET_TRUE )
		return true;
	
	return false;
}

int Calc_ExpGet_Exp(unsigned int level)
{
	int exp = 0;
	for ( int lev = 1; lev <= level; lev++ )
	{
		switch ( g_Cfg.m_iLevelMode )
		{
		case LEVEL_MODE_LINEAR:
			exp += g_Cfg.m_iLevelNextAt;
			break;
		case LEVEL_MODE_DOUBLE:
			exp += (g_Cfg.m_iLevelNextAt * (lev + 1));
			break;
		}
	}
	return exp;
}

int Calc_ExpGet_Level(unsigned int exp)
{
	for ( int lev = 0; ; lev++ )
	{
		unsigned int e = 0;
		switch ( g_Cfg.m_iLevelMode )
		{
		case LEVEL_MODE_LINEAR:
			e = g_Cfg.m_iLevelNextAt;
			break;
		case LEVEL_MODE_DOUBLE:
			e = (g_Cfg.m_iLevelNextAt * (lev + 1));
			break;
		}

		if ( exp < e )
			return lev;
		exp -= e;
	}
	return 0;
}

void CChar::ChangeExperience(int delta)
{
	static LPCTSTR const keyWords[] =
	{
		"a bit of",					// 0
		"a small amount of",
		"a little",
		"some",
		"a moderate amount of",
		"alot of",					// 5
		"large amounts of",
		"huge amounts of",
	};

	if ( !delta )
		goto levels;	//	zero call will sync the exp level

	if ( delta < 0 )
	{
		if ( !(g_Cfg.m_iExperienceMode&EXP_MODE_ALLOW_DOWN) )	// do not allow changes to minus
			return;

								// limiting delta to current level? check if delta goes out of level
		if ( g_Cfg.m_bLevelSystem && g_Cfg.m_iExperienceMode&EXP_MODE_DOWN_NOLEVEL )
		{
			int exp = Calc_ExpGet_Exp(m_level);
			if ( m_exp + delta < exp )
				delta = m_exp - exp;
		}
	}

	if ( g_Cfg.m_wDebugFlags&DEBUGF_EXP )
	{
		DEBUG_ERR(("%s %s experience change (was %d, delta %d, now %d)" DEBUG_CR,
			(m_pNPC ? "NPC" : "Player" ), GetName(), m_exp, delta, m_exp+delta));
	}

	if ( g_Cfg.m_iExperienceMode&EXP_MODE_TRIGGER_EXP )
	{
		CScriptTriggerArgs	args(delta);
		if ( OnTrigger(CTRIG_ExpChange, this, &args) == TRIGRET_RET_TRUE )
			return;
		delta = args.m_iN1;
	}
	m_exp += delta;

	if ( m_pClient )
	{
		const char *pWord = keyWords[0];
		int absval = abs(delta);
		int maxval = ( g_Cfg.m_bLevelSystem && g_Cfg.m_iLevelNextAt ) ? max(g_Cfg.m_iLevelNextAt,1000) : 1000;

		if ( absval >= maxval )				// 100%
			pWord = keyWords[7];
		else if ( absval >= (maxval*2)/3 )	//  66%
			pWord = keyWords[6];
		else if ( absval >= maxval/2 )		//  50%
			pWord = keyWords[5];
		else if ( absval >= maxval/3 )		//  33%
			pWord = keyWords[4];
		else if ( absval >= maxval/5 )		//  20%
			pWord = keyWords[3];
		else if ( absval >= maxval/7 )		//  14%
			pWord = keyWords[2];
		else if ( absval >= maxval/14 )		//   7%
			pWord = keyWords[1];

		m_pClient->SysMessagef("You have %s %s experience.",
			( delta > 0 ) ? "gained" : "lost",
			pWord);
	}

levels:
	if ( g_Cfg.m_bLevelSystem )
	{
		int level = Calc_ExpGet_Level(m_exp);

		if ( level != m_level )
		{
			int delta = level - m_level;

			if ( g_Cfg.m_iExperienceMode&EXP_MODE_TRIGGER_LEVEL )
			{
				CScriptTriggerArgs	args(delta);
				if ( OnTrigger(CTRIG_ExpLevelChange, this, &args) == TRIGRET_RET_TRUE )
					return;
				delta = args.m_iN1;
			}

			level = m_level + delta;
			if ( g_Cfg.m_wDebugFlags&DEBUGF_LEVEL )
			{
				DEBUG_ERR(("%s %s level change (was %d, delta %d, now %d)" DEBUG_CR,
					(m_pNPC ? "NPC" : "Player" ), GetName(), m_level, delta, level));
			}
			m_level = level;

			if ( m_pClient )
			{
				m_pClient->SysMessagef("You have %s %s level%s)",
					( delta > 0 ) ? "gained" : "lost",
					( abs(delta) == 1 ) ? "a" : "some",
					( abs(delta) > 1 ) ? "s" : "");
			}
		}
	}
}
