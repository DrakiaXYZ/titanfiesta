/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#include "stdafx.h"
#include "SHN Editor.h"
#include "CDlg.hpp"

CTitanListCtrl::CTitanListCtrl()
{
}

CTitanListCtrl::~CTitanListCtrl()
{
}

BEGIN_MESSAGE_MAP(CTitanListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfoList)
	ON_NOTIFY_REFLECT_EX(NM_DBLCLK, OnNMDblclk)
	ON_EN_KILLFOCUS(1337, HideEditor)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_RCLICK_SHOWINHEX, ShowColumnInHex)
END_MESSAGE_MAP()

BOOL CTitanListCtrl::EnsureSubItemVisible(int nItem, int nSubItem, CRect *pRect){
	BOOL ret = EnsureVisible(nItem, FALSE);
	CRect rect;
	GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rect);
	CRect rtList;
	GetClientRect(&rtList);
	if(rect.right > rtList.Width()) Scroll(CSize( rect.Width() > rtList.Width()?rect.left : rect.right - rtList.Width(), 0));
	if(rect.left < 0) Scroll(CSize(rect.left));
	if(pRect){
		GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rect);
		rect.right = min(rect.right, rtList.Width()-4);
		*pRect = rect;
	}
	return ret;
}

void CTitanListCtrl::ShowColumnInHex(){
	if(curFile == NULL) return;
//	curFile->columns[curEditColumn]->showInHex = !curFile->columns[curEditColumn]->showInHex;
//	RedrawItems(0, curFile->rows.size());
}

void CTitanListCtrl::OnContextMenu(CWnd* pWnd, CPoint point){
	if(curFile == NULL) return;
	ScreenToClient(&point);
	LVHITTESTINFO lvhti;
	ZeroMemory(&lvhti, sizeof(lvhti));
	lvhti.pt = point;
	HitTest(&lvhti);
	ListView_SubItemHitTest(m_hWnd, &lvhti);
	int item = lvhti.iItem;
	int subItem = lvhti.iSubItem;
	if(subItem >= 0){
		curEditColumn = subItem;
		CMenu menu;
		menu.LoadMenu(IDR_MENU_COLUMN_HEADER);
		ClientToScreen(&point);
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,this);
	}
}

BOOL CTitanListCtrl::DisplayEditor(int nItem, int nSubItem){
	if(curFile == NULL) return TRUE;

	CRect rectSubItem;
	EnsureSubItemVisible(nItem, nSubItem, &rectSubItem);

	m_wndEdit.SetWindowText(GetItemText(nItem, nSubItem));
	m_wndEdit.SetWindowPos(NULL, rectSubItem.left, rectSubItem.top, rectSubItem.Width(), rectSubItem.Height(), SWP_SHOWWINDOW);	
	m_wndEdit.SetFocus();
	
	nCurEditItem = nItem;
	nCurEditSubItem = nSubItem;

	return TRUE;
}

void CTitanListCtrl::HideEditor(){
/*	if(curFile == NULL) return;
	CShnRow* curRow = curFile->Row(nCurEditItem);
	CShnData* curData = curRow->data[nCurEditSubItem];
	CShnColumn* curCol = curFile->Column(nCurEditSubItem);

	CString wndTxt;
	m_wndEdit.GetWindowText(wndTxt);
	switch(curCol->type){
		case 1:
			curData->data[0] = atoi(wndTxt);
		break;
		case 2:
			*reinterpret_cast<word*>(curData->data) = atoi(wndTxt);
		break;
		case 3:
		case 11:
		case 18:
		case 27:
			*reinterpret_cast<dword*>(curData->data) = atoi(wndTxt);
		break;
		case 9:
		case 26:
			DEL(curData->data);
			curData->data = new byte[wndTxt.GetLength() + 1];
			strcpy_s(reinterpret_cast<char*>(curData->data), wndTxt.GetLength() + 1, wndTxt);
		break;
	}*/
	m_wndEdit.ShowWindow(false);
}

BOOL CTitanListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;
	if(!pNMListView) return FALSE;
	int nItem = pNMListView->iItem, nSubItem = pNMListView->iSubItem;		

	*pResult = DisplayEditor(nItem, nSubItem);

	return *pResult;
}

BOOL CTitanListCtrl::PreTranslateMessage(MSG* pMsg){
	if(!IsWindow(m_wndEdit.m_hWnd))
		m_wndEdit.Create(WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, CRect(10, 10, 100, 100), this, 1337);

	if(pMsg->message == WM_KEYDOWN){
		if(pMsg->wParam == VK_RETURN){
			m_wndEdit.ShowWindow(FALSE);
			return TRUE;
		}
	}else if(pMsg->message == WM_MOUSEWHEEL || pMsg->message == WM_VSCROLL || pMsg->message == WM_HSCROLL){
		m_wndEdit.ShowWindow(FALSE);
	}
	return CListCtrl::PreTranslateMessage(pMsg);
}

void CTitanListCtrl::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult){
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;

	int itemid = pItem->iItem;
	if(pItem->mask & LVIF_TEXT && curFile != NULL){
		CString text;
		if(pItem->iItem >= curFile->RowCount()) return;
		if(pItem->iSubItem >= curFile->ColCount()) return;
		CShnRow* curRow = curFile->Row(pItem->iItem);
		CShnData* curData = curRow->cells[pItem->iSubItem];
		CShnColumn* curCol = curFile->Column(pItem->iSubItem);
		switch(curCol->type){
			case 1:
				//if(!curCol->showInHex)
					sprintf_s(pItem->pszText, pItem->cchTextMax, "%d", curData->btData);
				/*else
					sprintf_s(pItem->pszText, pItem->cchTextMax, "%02x", curData->data[0]);*/
			break;
			case 2:
				//if(!curCol->showInHex)
					sprintf_s(pItem->pszText, pItem->cchTextMax, "%d", curData->wData);
				/*else
					sprintf_s(pItem->pszText, pItem->cchTextMax, "%04x", *reinterpret_cast<word*>(curData->data));*/
			break;
			case 3:
			case 11:
			case 18:
			case 27:
				//if(!curCol->showInHex)
					sprintf_s(pItem->pszText, pItem->cchTextMax, "%d", curData->dwData);
				/*else
					sprintf_s(pItem->pszText, pItem->cchTextMax, "%08x", *reinterpret_cast<dword*>(curData->data));*/
			break;
			case 9:
			case 26:
				sprintf_s(pItem->pszText, pItem->cchTextMax, "%s", curData->strData);
			break;
		}
	}

	*pResult = 0;
}
