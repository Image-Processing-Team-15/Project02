
// ImageProcessingView.cpp : CImageProcessingView 클래스의 구현
//

#include "stdafx.h"
#include "ImageProcessing.h"

#include "MainFrm.h"
#include "ImageProcessingDoc.h"
#include "ImageProcessingView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CImageProcessingView

IMPLEMENT_DYNCREATE(CImageProcessingView, CScrollView)

BEGIN_MESSAGE_MAP(CImageProcessingView, CScrollView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CScrollView::OnFilePrintPreview)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CImageProcessingView 생성/소멸

CImageProcessingView::CImageProcessingView()
{
	// TODO: 여기에 생성 코드를 추가합니다.

}

CImageProcessingView::~CImageProcessingView()
{
}

BOOL CImageProcessingView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CScrollView::PreCreateWindow(cs);
}

// CImageProcessingView 그리기

void CImageProcessingView::OnDraw(CDC* pDC)
{
	CImageProcessingDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
	if(pDoc->GetImage())
	{
		pDoc->GetImage()->Draw(pDC->GetSafeHdc());
	}
}

void CImageProcessingView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);


	CImageProcessingDoc* pDoc = GetDocument();
	CxImage* ima = pDoc->GetImage();
	if (ima) {
		int px=GetScrollPos(SB_HORZ);
		int py=GetScrollPos(SB_VERT);
		CSize sz(GetTotalSize());
		int x=(int)(ima->GetWidth());
		int y=(int)(ima->GetHeight());
		SetScrollSizes(MM_TEXT,	CSize(x,y));
		CSize sz2(GetTotalSize());

		CWnd* pFrame=GetParentFrame();
		RECT rClient;
		pFrame->GetClientRect(&rClient);

		if (sz.cx!=0 && sz.cy!=0){
			if (x>rClient.right) SetScrollPos(SB_HORZ,sz2.cx*px/sz.cx); else  SetScrollPos(SB_HORZ,0);
			if (y>rClient.bottom) SetScrollPos(SB_VERT,sz2.cy*py/sz.cy); else SetScrollPos(SB_VERT,0);
		}

		if (!(pFrame->IsIconic()||pFrame->IsZoomed())){
			RECT rMainCl,rFrame,rMainFr;
			(AfxGetApp()->m_pMainWnd)->GetClientRect(&rMainCl);
			(AfxGetApp()->m_pMainWnd)->GetWindowRect(&rMainFr);
			pFrame->GetWindowRect(&rFrame);
			pFrame->SetWindowPos(0,0,0,
				(4+rFrame.right-rFrame.left-rClient.right+rClient.left)+
				min(rMainCl.right-(rFrame.left-rMainFr.left+12),x),
				(4+rFrame.bottom-rFrame.top-rClient.bottom+rClient.top)+
				min(rMainCl.bottom-(rFrame.top-rMainFr.top+12),y),
				SWP_NOMOVE|SWP_NOZORDER);
		}

		CMainFrame* pMain = (CMainFrame*) AfxGetMainWnd();
	}
}

void CImageProcessingView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	CImageProcessingDoc* pDoc = GetDocument();
	if (pDoc){
		CxImage*  ima  = pDoc->GetImage();
		if (ima){
			SetScrollSizes(MM_TEXT,CSize((int)(ima->GetWidth()), (int)(ima->GetHeight())));
		}
	}


	CScrollView::OnUpdate(pSender, lHint, pHint);
}

void CImageProcessingView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{

	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}


// CImageProcessingView 인쇄

BOOL CImageProcessingView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CImageProcessingView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CImageProcessingView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CImageProcessingView 진단

#ifdef _DEBUG
void CImageProcessingView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CImageProcessingView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CImageProcessingDoc* CImageProcessingView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CImageProcessingDoc)));
	return (CImageProcessingDoc*)m_pDocument;
}
#endif //_DEBUG


// CImageProcessingView 메시지 처리기



void CImageProcessingView::OnDestroy()
{
	CScrollView::OnDestroy();

}


