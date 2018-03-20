// PHNPAN.CPP
// Copyright (c) A.Sobolev 2018
//
#include <pp.h>
#pragma hdrstop

class PhonePaneDialog : public TDialog {
public:
	struct State {
		enum {
			lmNone = 0,
			lmBill,
			lmTask,
			lmPersonEvent,
			lmScOp,
			lmScCCheck,  // ���� �� �����
			lmLocCCheck, // ���� �� ����������� ������
			lmSwitchTo
		};
		SLAPI  State() : Mode(lmNone)
		{
		}
		long   Mode;
		SString Channel;
		SString CallerID;
		SString ConnectedLine;
		//
		PPObjIDArray RelEntries;
	};
	PhonePaneDialog(PhoneServiceEventResponder * pPSER, const PhonePaneDialog::State * pSt);
	static PhonePaneDialog * FindAnalogue(const char * pChannel)
	{
		const long res_id = DLG_PHNCPANE;
		for(TView * p = APPL->P_DeskTop->GetFirstView(); p != 0; p = p->nextView()) {
			if(p->IsConsistent() && p->GetSubSign() == TV_SUBSIGN_DIALOG && ((TDialog *)p)->resourceID == res_id)
				return (PhonePaneDialog *)p;
		}
		return 0;
	}
private:
	DECL_HANDLE_EVENT
	{
		TDialog::handleEvent(event);
	}
	void   OnContactSelection();
	State  S;
	PhoneServiceEventResponder * P_PSER;
	PPObjPerson PsnObj;
	PPObjSCard ScObj;
	PPObjIDArray OidList;
};

void PhonePaneDialog::OnContactSelection()
{
	long   item_id = getCtrlLong(CTLSEL_PHNCPANE_NAME);
	/*AddClusterAssocDef(CTL_PHNCPANE_LISTMODE, 0, State::lmSwitchTo);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 1, State::lmBill);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 2, State::lmTask);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 3, State::lmPersonEvent);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 4, State::lmScOp);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 5, State::lmScCCheck);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 6, State::lmLocCCheck);*/
	if(item_id && item_id > 0 && item_id <= OidList.getCount()) {
		const PPObjID & r_oid = OidList.at(item_id-1);
		if(r_oid.Obj == PPOBJ_PERSON) {
			DisableClusterItem(CTL_PHNCPANE_LISTMODE, 2, 0);
			DisableClusterItem(CTL_PHNCPANE_LISTMODE, 3, 0);
			DisableClusterItem(CTL_PHNCPANE_LISTMODE, 4, 1);
			DisableClusterItem(CTL_PHNCPANE_LISTMODE, 5, 1);
			DisableClusterItem(CTL_PHNCPANE_LISTMODE, 6, 1);
		}
		else if(r_oid.Obj == PPOBJ_LOCATION) {
		}
		else if(r_oid.Obj == PPOBJ_SCARD) {
		}
	}
	else {
		DisableClusterItem(CTL_PHNCPANE_LISTMODE, 1, 1);
		DisableClusterItem(CTL_PHNCPANE_LISTMODE, 2, 1);
		DisableClusterItem(CTL_PHNCPANE_LISTMODE, 3, 1);
		DisableClusterItem(CTL_PHNCPANE_LISTMODE, 4, 1);
		DisableClusterItem(CTL_PHNCPANE_LISTMODE, 5, 1);
		DisableClusterItem(CTL_PHNCPANE_LISTMODE, 6, 1);
	}
}

PhonePaneDialog::PhonePaneDialog(PhoneServiceEventResponder * pPSER, const PhonePaneDialog::State * pSt) : TDialog(DLG_PHNCPANE), P_PSER(pPSER)
{
	RVALUEPTR(S, pSt);
	SString temp_buf;
	setCtrlString(CTL_PHNCPANE_PHN, S.ConnectedLine);
	temp_buf = S.Channel;
	if(S.ConnectedLine.NotEmpty())
		temp_buf.CatDiv(';', 2).Cat(S.ConnectedLine);
	setStaticText(CTL_PHNCPANE_ST_INFO, temp_buf);
	AddClusterAssocDef(CTL_PHNCPANE_LISTMODE, 0, State::lmSwitchTo);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 1, State::lmBill);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 2, State::lmTask);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 3, State::lmPersonEvent);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 4, State::lmScOp);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 5, State::lmScCCheck);
	AddClusterAssoc(CTL_PHNCPANE_LISTMODE, 6, State::lmLocCCheck);
	{
		StrAssocArray name_list;
		PPID   init_id = 0;
		SString list_item_buf;
		P_PSER->IdentifyCaller(S.ConnectedLine, OidList);
		for(uint i = 0; i < OidList.getCount(); i++) {
			const PPObjID & r_oid = OidList.at(i);
			list_item_buf.Z();
			if(r_oid.Obj == PPOBJ_PERSON) {
				GetPersonName(r_oid.Id, temp_buf);
				GetObjectTitle(r_oid.Obj, list_item_buf);
				list_item_buf.CatDiv(':', 2).Cat(temp_buf);
			}
			else if(r_oid.Obj == PPOBJ_LOCATION) {
				LocationTbl::Rec loc_rec;
				if(PsnObj.LocObj.Search(r_oid.Id, &loc_rec) > 0) {
					LocationCore::GetExField(&loc_rec, LOCEXSTR_CONTACT, temp_buf);
					GetObjectTitle(r_oid.Obj, list_item_buf);
					list_item_buf.CatDiv(':', 2).Cat(temp_buf);
				}
			}
			else if(r_oid.Obj == PPOBJ_SCARD) {
				SCardTbl::Rec sc_rec;
				if(ScObj.Search(r_oid.Id, &sc_rec) > 0) {
					GetObjectTitle(r_oid.Obj, list_item_buf);
					list_item_buf.CatDiv(':', 2).Cat(sc_rec.Code);
					PersonTbl::Rec psn_rec;
					if(sc_rec.PersonID && PsnObj.Search(sc_rec.PersonID, &psn_rec) > 0) {
						list_item_buf.Space().Cat(psn_rec.Name);
					}
				}
			}
			if(list_item_buf.NotEmpty()) {
				name_list.Add(i+1, list_item_buf);
				if(init_id == 0)
					init_id = i+1;
			}
		}
		SetupStrAssocCombo(this, CTLSEL_PHNCPANE_NAME, &name_list, init_id, 0, 0, 0);
	}
}

int SLAPI ShowPhoneCallPane(PhoneServiceEventResponder * pPSER, const PhonePaneDialog::State * pSt)
{
	int    ok = 1;
	PhonePaneDialog * p_prev_dlg = PhonePaneDialog::FindAnalogue("");
	if(p_prev_dlg) {
		ok = -1;
	}
	else {
		PhonePaneDialog * p_dlg = new PhonePaneDialog(pPSER, pSt);
		if(CheckDialogPtr(&p_dlg)) {
			APPL->P_DeskTop->Insert_(p_dlg);
			p_dlg->Insert();
		}
	}
	return ok;
}

SLAPI PhoneServiceEventResponder::PhoneServiceEventResponder() : AdvCookie_Ringing(0), AdvCookie_Up(0), P_PsnObj(0)
{
	{
		PPAdviseBlock adv_blk;
		adv_blk.Kind = PPAdviseBlock::evPhoneRinging;
		adv_blk.ProcExtPtr = this;
		adv_blk.Proc = PhoneServiceEventResponder::AdviseCallback;
		DS.Advise(&AdvCookie_Ringing, &adv_blk);
	}
	{
		PPAdviseBlock adv_blk;
		adv_blk.Kind = PPAdviseBlock::evPhoneUp;
		adv_blk.ProcExtPtr = this;
		adv_blk.Proc = PhoneServiceEventResponder::AdviseCallback;
		DS.Advise(&AdvCookie_Up, &adv_blk);
	}
}

SLAPI PhoneServiceEventResponder::~PhoneServiceEventResponder()
{
	DS.Unadvise(AdvCookie_Ringing);
	DS.Unadvise(AdvCookie_Up);
	ZDELETE(P_PsnObj);
}

int SLAPI PhoneServiceEventResponder::IdentifyCaller(const char * pCaller, PPObjIDArray & rList)
{
	rList.clear();
	int    ok = -1;
	SString caller_buf;
	PPObjLocation loc_obj;
	PPIDArray ea_id_list;
	PPEAddr::Phone::NormalizeStr(pCaller, caller_buf);
	THROW_SL(SETIFZ(P_PsnObj, new PPObjPerson));
	P_PsnObj->LocObj.P_Tbl->SearchPhoneIndex(caller_buf, 0, ea_id_list);
	if(ea_id_list.getCount()) {
		for(uint i = 0; i < ea_id_list.getCount(); i++) {
			EAddrTbl::Rec ea_rec;
			if(loc_obj.P_Tbl->GetEAddr(ea_id_list.get(i), &ea_rec) > 0) {
				rList.Add(ea_rec.LinkObjType, ea_rec.LinkObjID);
				ok = 1;
			}
		}
	}
	CATCHZOK
	return ok;
}

/*
int PhoneServiceEventResponder::IdentifyCaller(const PPNotifyEvent * pEv)
{
	if(status_list.GetCount()) {
		PPEAddrArray phn_list; // ������ ���������, ����������� � ������. ��������� ��� ���������� ����������� �����.
		PPIDArray ea_id_list;
		SString contact_buf;
		for(uint i = 0; !pop_dlvr_pane && i < status_list.GetCount(); i++) {
			status_list.Get(i, cnl_status);
			if(cnl_status.State == PhnSvcChannelStatus::stUp) {
				if(cnl_status.Channel.CmpPrefix("SIP", 1) == 0) {
					if(cnl_status.ConnectedLineNum.Empty() || cnl_status.ConnectedLineNum.ToLong() != 0) {
						if(PPObjPhoneService::IsPhnChannelAcceptable(PhnSvcLocalChannelSymb, cnl_status.Channel)) {
							if(CnSpeciality == PPCashNode::spDelivery && !(P.Eccd.Flags & P.Eccd.fDelivery) && IsState(sEMPTYLIST_EMPTYBUF) && !(Flags & fBarrier)) {
								pop_dlvr_pane = 1;
								if(cnl_status.ConnectedLineNum.Len() > cnl_status.CallerId.Len())
									phone_buf = cnl_status.ConnectedLineNum;
								else
									phone_buf = cnl_status.CallerId;
								channel_buf = cnl_status.Channel;
							}
						}
					}
				}
			}
			else if(cnl_status.State == PhnSvcChannelStatus::stRinging) {
				if(cnl_status.ConnectedLineNum.Len() > cnl_status.CallerId.Len())
					caller_buf = cnl_status.ConnectedLineNum;
				else
					caller_buf = cnl_status.CallerId;
				if(caller_buf.Len() && !phn_list.SearchPhone(caller_buf, 0, 0)) {
					if(ringing_line.NotEmpty())
						ringing_line.CR();
					ringing_line.Cat(cnl_status.Channel).CatDiv(':', 2).Cat(caller_buf);
					phn_list.AddPhone(caller_buf);
					PsnObj.LocObj.P_Tbl->SearchPhoneIndex(caller_buf, 0, ea_id_list);
					contact_buf.Z();
					for(uint j = 0; !contact_buf.NotEmpty() && j < ea_id_list.getCount(); j++) {
						EAddrTbl::Rec ea_rec;
						if(PsnObj.LocObj.P_Tbl->GetEAddr(ea_id_list.get(j), &ea_rec) > 0) {
							if(ea_rec.LinkObjType == PPOBJ_PERSON) {
								GetPersonName(ea_rec.LinkObjID, contact_buf);
							}
							else if(ea_rec.LinkObjType == PPOBJ_LOCATION) {
								LocationTbl::Rec loc_rec;
								if(PsnObj.LocObj.Search(ea_rec.LinkObjID, &loc_rec) > 0)
									LocationCore::GetExField(&loc_rec, LOCEXSTR_CONTACT, contact_buf);
							}
						}
					}
					if(contact_buf.Empty())
						contact_buf = "UNKNOWN";
					ringing_line.CatDiv(';', 2).Cat(contact_buf);
				}
			}
		}
	}
}
*/

//static 
int PhoneServiceEventResponder::AdviseCallback(int kind, const PPNotifyEvent * pEv, void * procExtPtr)
{
	int    ok = -1;
	SString msg_buf;
	SString temp_buf;
	SString caller;
	SString channel;
	SString connected_line;
	if(kind == PPAdviseBlock::evPhoneRinging) {
		PhoneServiceEventResponder * p_self = (PhoneServiceEventResponder *)procExtPtr;
		if(p_self) {
			(msg_buf = "PhoneRinging").CatDiv(':', 2);
			pEv->GetExtStrData(pEv->extssChannel, channel);
			msg_buf.CatEq("channel", channel).CatDiv(';', 2);
			pEv->GetExtStrData(pEv->extssCallerId, caller);
			msg_buf.CatEq("callerid", caller).CatDiv(';', 2);
			pEv->GetExtStrData(pEv->extssConnectedLineNum, connected_line);
			msg_buf.CatEq("connectedline", connected_line);
			PPLogMessage(PPFILNAM_DEBUG_LOG, msg_buf, LOGMSGF_TIME);
			if(connected_line.NotEmpty()) {
				SMessageWindow * p_win = new SMessageWindow;
				if(p_win) {
					SString contact_buf;
					PPObjIDArray identified_caller_list;
					if(p_self->IdentifyCaller(connected_line, identified_caller_list) > 0) {
						for(uint i = 0; contact_buf.Empty() && i < identified_caller_list.getCount(); i++) {
							const PPObjID & r_oid = identified_caller_list.at(i);
							if(r_oid.Obj == PPOBJ_PERSON) {
								GetPersonName(r_oid.Id, contact_buf);
							}
							else if(r_oid.Obj == PPOBJ_LOCATION) {
								LocationTbl::Rec loc_rec;
								if(p_self->P_PsnObj->LocObj.Search(r_oid.Id, &loc_rec) > 0)
									LocationCore::GetExField(&loc_rec, LOCEXSTR_CONTACT, contact_buf);
							}
						}
					}
					SString fmt_buf;
					PPLoadText(PPTXT_YOUARETELEPHONED, fmt_buf);
					temp_buf = connected_line;
					if(contact_buf.NotEmpty()) {
						temp_buf.Space().CatParStr(contact_buf);
					}
					msg_buf.Printf(fmt_buf, temp_buf.cptr());
					p_win->Open(msg_buf, 0, /*H()*/0, 0, 5000, GetColorRef(SClrCadetblue),
						SMessageWindow::fTopmost|SMessageWindow::fSizeByText|SMessageWindow::fPreserveFocus, 0);
				}
			}
			ok = 1;
		}
	}
	else if(kind == PPAdviseBlock::evPhoneUp) {
		PhoneServiceEventResponder * p_self = (PhoneServiceEventResponder *)procExtPtr;
		if(p_self) {
			(msg_buf = "PhoneUp").CatDiv(':', 2);
			pEv->GetExtStrData(pEv->extssChannel, channel);
			msg_buf.CatEq("channel", channel).CatDiv(';', 2);
			pEv->GetExtStrData(pEv->extssCallerId, caller);
			msg_buf.CatEq("callerid", caller).CatDiv(';', 2);
			pEv->GetExtStrData(pEv->extssConnectedLineNum, connected_line);
			msg_buf.CatEq("connectedline", connected_line);
			PPLogMessage(PPFILNAM_DEBUG_LOG, msg_buf, LOGMSGF_TIME);
			{
				PhonePaneDialog::State state;
				state.CallerID = caller;
				state.Channel = channel;
				state.ConnectedLine = connected_line;
				p_self->IdentifyCaller(caller, state.RelEntries);
				ShowPhoneCallPane(p_self, &state);
			}
			ok = 1;
		}
	}
	return ok;
}
//
// @ModuleDef(PPViewJobPool)
//
//Event: Status;Privilege: Call;Channel: SIP/198-0000027c;ChannelState: 6;ChannelStateDesc: Up;CallerIDNum: 198;CallerIDName: Sobolev (soft sip);
	// ConnectedLineNum: 110;ConnectedLineName: ������� �����;Accountcode: ;Context: from-internal;Exten: ;Priority: 1;Uniqueid: 1520526041.660;Type: SIP;DNID: ;
	// EffectiveConnectedLineNum: 110;EffectiveConnectedLineName: ������� �����;TimeToHangup: 0;BridgeID: 89e33c3e-f276-45a3-b66e-b0a5fda6fa4e;
	// Linkedid: 1520526041.659;Application: AppDial;Data: (Outgoing Line);Nativeformats: (ulaw);Readformat: ulaw;Readtrans: ;Writeformat: ulaw;
	// Writetrans: ;Callgroup: 0;Pickupgroup: 0;Seconds: 12;ActionID: 2899;;
//Event: Status;Privilege: Call;Channel: SIP/110-0000027b;ChannelState: 6;ChannelStateDesc: Up;CallerIDNum: 110;CallerIDName: ������� �����;ConnectedLineNum: 198;ConnectedLineName: Sobolev (soft sip);Accountcode: ;Context: macro-dial-one;Exten: s;Priority: 52;Uniqueid: 1520526041.659;Type: SIP;DNID: 198;EffectiveConnectedLineNum: 198;EffectiveConnectedLineName: Sobolev (soft sip);TimeToHangup: 0;BridgeID: 89e33c3e-f276-45a3-b66e-b0a5fda6fa4e;Linkedid: 1520526041.659;Application: Dial;Data: SIP/198,,TtrIb(func-apply-sipheaders^s^1);Nativeformats: (ulaw);Readformat: ulaw;Readtrans: ;Writeformat: ulaw;Writetrans: ;Callgroup: 0;Pickupgroup: 0;Seconds: 12;ActionID: 2899;;

IMPLEMENT_PPFILT_FACTORY(PhnSvcMonitor); SLAPI PhnSvcMonitorFilt::PhnSvcMonitorFilt() : PPBaseFilt(PPFILT_PHNSVCMONITOR, 0, 0)
{
	SetFlatChunk(offsetof(PhnSvcMonitorFilt, ReserveStart),
		offsetof(PhnSvcMonitorFilt, ReserveEnd)-offsetof(PhnSvcMonitorFilt, ReserveStart)+sizeof(ReserveEnd));
	Init(1, 0);
}

SLAPI PPViewPhnSvcMonitor::PPViewPhnSvcMonitor() : PPView(0, &Filt, PPVIEW_PHNSVCMONITOR), P_Cli(0)
{
	ImplementFlags |= implBrowseArray;
}

SLAPI PPViewPhnSvcMonitor::~PPViewPhnSvcMonitor()
{
}

PPBaseFilt * SLAPI PPViewPhnSvcMonitor::CreateFilt(void * extraPtr) const
{
	PhnSvcMonitorFilt * p_filt = new PhnSvcMonitorFilt;
	{
		PPEquipConfig eq_cfg;
		ReadEquipConfig(&eq_cfg);
		p_filt->PhnSvcID = eq_cfg.PhnSvcID;
	}
	return p_filt;
}

int SLAPI PPViewPhnSvcMonitor::EditBaseFilt(PPBaseFilt * pBaseFilt)
{
	return 1;
}

int SLAPI PPViewPhnSvcMonitor::CreatePhnSvcClient()
{
	int    ok = 1;
	SString temp_buf;
	ZDELETE(P_Cli);
	THROW(Filt.PhnSvcID);
	{
		PPObjPhoneService ps_obj(0);
		PPPhoneServicePacket ps_pack;
		THROW(ps_obj.GetPacket(Filt.PhnSvcID, &ps_pack) > 0);
		{
			SString addr_buf, user_buf, secret_buf;
			AsteriskAmiClient * p_phnsvc_cli = 0;
			ps_pack.GetExField(PHNSVCEXSTR_ADDR, addr_buf);
			ps_pack.GetExField(PHNSVCEXSTR_PORT, temp_buf);
			int    port = temp_buf.ToLong();
			ps_pack.GetExField(PHNSVCEXSTR_USER, user_buf);
			ps_pack.GetPassword(secret_buf);
			THROW_MEM(p_phnsvc_cli = new AsteriskAmiClient(AsteriskAmiClient::fDoLog));
			THROW(p_phnsvc_cli->Connect(addr_buf, port));
			THROW(p_phnsvc_cli->Login(user_buf, secret_buf));
			P_Cli = p_phnsvc_cli;
			secret_buf.Obfuscate();
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPViewPhnSvcMonitor::Init_(const PPBaseFilt * pBaseFilt)
{
	int    ok = 1;
	THROW(Helper_InitBaseFilt(pBaseFilt));
	ZDELETE(P_Cli);
	THROW(CreatePhnSvcClient());
	THROW(Update());
	CATCHZOK
	return ok;
}

int SLAPI PPViewPhnSvcMonitor::Update()
{
	int    ok = -1;
	if(P_Cli) {
		int r = P_Cli->GetChannelStatus(0, List);
		if(!r) {
			r = CreatePhnSvcClient();
			if(r) {
				assert(P_Cli);
				P_Cli->GetChannelStatus(0, List);
			}
		}
	}
	else
		List.Clear();
	return ok;
}

void SLAPI PPViewPhnSvcMonitor::PreprocessBrowser(PPViewBrowser * pBrw)
{
	if(pBrw) {
		pBrw->SetDefUserProc(PPViewPhnSvcMonitor::GetDataForBrowser, this);
		pBrw->SetRefreshPeriod(1);
		//pBrw->SetCellStyleFunc(CellStyleFunc, pBrw);
	}
}

int SLAPI PPViewPhnSvcMonitor::_GetDataForBrowser(SBrowserDataProcBlock * pBlk)
{
	int    ok = 0;
	if(pBlk->P_SrcData && pBlk->P_DestData) {
		ok = 1;
		uint   _pos = *(uint *)pBlk->P_SrcData;
		if(_pos > 0 && _pos <= List.GetCount()) {
			List.Get(_pos-1, TempStatusEntry);
			switch(pBlk->ColumnN) {
				case 0: // @id
					pBlk->Set((long)_pos);
					break;
				case 1: // Channel
					pBlk->Set(TempStatusEntry.Channel);
					break;
				case 2: // State
					{
						SString & r_temp_buf = SLS.AcquireRvlStr();
						AsteriskAmiClient::GetStateText(TempStatusEntry.State, r_temp_buf);
						pBlk->Set(r_temp_buf);
					}
					break;
				case 3: // Priority
					pBlk->Set(TempStatusEntry.Priority);
					break;
				case 4: // Seconds
					pBlk->Set(TempStatusEntry.Seconds);
					break;
				case 5: // TimeToHungUp
					pBlk->Set(TempStatusEntry.TimeToHungUp);
					break;
				case 6: // CallerId
					pBlk->Set(TempStatusEntry.CallerId);
					break;
				case 7: // CallerName
					pBlk->Set(TempStatusEntry.CallerIdName.Transf(CTRANSF_UTF8_TO_INNER));
					break;
				case 8: // ConnectedLineNum
					pBlk->Set(TempStatusEntry.ConnectedLineNum);
					break;
				case 9: // ConnectedLineName
					pBlk->Set(TempStatusEntry.ConnectedLineName.Transf(CTRANSF_UTF8_TO_INNER));
					break;
				case 10: // EffConnectedLineNum
					pBlk->Set(TempStatusEntry.EffConnectedLineNum);
					break;
				case 11: // EffConnectedLineName
					pBlk->Set(TempStatusEntry.EffConnectedLineName.Transf(CTRANSF_UTF8_TO_INNER));
					break;
				case 12: // Context
					pBlk->Set(TempStatusEntry.Context.Transf(CTRANSF_UTF8_TO_INNER));
					break;
				case 13: // Exten
					pBlk->Set(TempStatusEntry.Exten);
					break;
				case 14: // DnId
					pBlk->Set(TempStatusEntry.DnId);
					break;
				case 15: // Application
					pBlk->Set(TempStatusEntry.Application.Transf(CTRANSF_UTF8_TO_INNER));
					break;
				case 16: // Data
					pBlk->Set(TempStatusEntry.Data.Transf(CTRANSF_UTF8_TO_INNER));
					break;
			}
		}
	}
	return ok;
}

//static 
int SLAPI PPViewPhnSvcMonitor::GetDataForBrowser(SBrowserDataProcBlock * pBlk)
{
	PPViewPhnSvcMonitor * p_v = (PPViewPhnSvcMonitor *)pBlk->ExtraPtr;
	return p_v ? p_v->_GetDataForBrowser(pBlk) : 0;
}

SArray * SLAPI PPViewPhnSvcMonitor::CreateBrowserArray(uint * pBrwId, SString * pSubTitle)
{
	uint   brw_id = BROWSER_PHNSVCMONITOR;
	SArray * p_array = new TSArray <uint>; // Array - not Vector
	if(p_array) {
		for(uint i = 0; i < List.GetCount(); i++) {
			uint   pos = i+1;
			p_array->insert(&pos);
		}
	}
	ASSIGN_PTR(pBrwId, brw_id);
	return p_array;
}

int SLAPI PPViewPhnSvcMonitor::ProcessCommand(uint ppvCmd, const void * pHdr, PPViewBrowser * pBrw)
{
	int    ok = PPView::ProcessCommand(ppvCmd, pHdr, pBrw);
	if(ok == -2 && oneof2(ppvCmd, PPVCMD_REFRESHBYPERIOD, PPVCMD_REFRESH)) {
		Update();
		AryBrowserDef * p_def = (AryBrowserDef *)pBrw->getDef();
		if(p_def) {
			SArray * p_array = CreateBrowserArray(0, 0);
			p_def->setArray(p_array, 0, 0);
		}
		ok = 1;
	}
	return ok;
}