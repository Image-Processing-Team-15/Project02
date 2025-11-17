
// ImageProcessing.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
// TODO: Visual Studio 2008 SP1 전용기능용 헤더 제거 // DONE
//#include "afxwinappex.h" 
#include "ImageProcessing.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "ImageProcessingDoc.h"
#include "ImageProcessingView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CImageProcessingApp

	BEGIN_MESSAGE_MAP(CImageProcessingApp, CWinApp)
	// 표준 파일을 기초로 하는 문서 명령입니다.
	ON_COMMAND(ID_FILE_OPEN, &CImageProcessingApp::OnFileOpen)
	// 표준 인쇄 설정 명령입니다.
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


// CImageProcessingApp 생성

CImageProcessingApp::CImageProcessingApp()
{

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

// 유일한 CImageProcessingApp 개체입니다.

CImageProcessingApp theApp;


// CImageProcessingApp 초기화

BOOL CImageProcessingApp::InitInstance()
{
	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다. 
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("VIM")); // TODO: 레지스트리 키 수정 // DONE
	LoadStdProfileSettings(4); // MRU를 포함하여 표준 INI 파일 옵션을 로드합니다.

	// 응용 프로그램의 문서 템플릿을 등록합니다. 문서 템플릿은
	//  문서, 프레임 창 및 뷰 사이의 연결 역할을 합니다.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_ImageProcessingTYPE,
		RUNTIME_CLASS(CImageProcessingDoc),
		RUNTIME_CLASS(CChildFrame), // 사용자 지정 MDI 자식 프레임입니다.
		RUNTIME_CLASS(CImageProcessingView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// 주 MDI 프레임 창을 만듭니다.
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// 접미사가 있을 경우에만 DragAcceptFiles를 호출합니다.
	//  MDI 응용 프로그램에서는 m_pMainWnd를 설정한 후 바로 이러한 호출이 발생해야 합니다.


	// 표준 셸 명령, DDE, 파일 열기에 대한 명령줄을 구문 분석합니다.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// TODO: 새창열기 명령 무효화 // DONE
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;


	// 명령줄에 지정된 명령을 디스패치합니다.
	// 응용 프로그램이 /RegServer, /Register, /Unregserver 또는 /Unregister로 시작된 경우 FALSE를 반환합니다.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 주 창이 초기화되었으므로 이를 표시하고 업데이트합니다.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}




// CImageProcessingApp 메시지 처리기

// TODO: 파일 열기 재정의 // DONE
void CImageProcessingApp::OnFileOpen() 
{
	static TCHAR BASED_CODE szFilter[] = 
		_T("Supported Files(*.bmp;*.gif;*.jpg;*.jpeg;*.png;*.ico;*.tif;*.tiff;*.tga;*.pcx;*.wbmp;*.j2k;*.jp2;;*.j2c;*.jpc;*.pgx;*.pnm;*.pgm;*.ppm;*.ras;*.mng;*.jng;*.ska;*.nef;*.crw;*.cr2;*.mrw;*.raf;*.erf;*.3fr;*.dcr;*.raw;*.dng;*.pef;*.x3f;*.arw;*.sr2;*.mef;*.orf)|*.bmp;*.gif;*.jpg;*.jpeg;*.png;*.ico;*.tif;*.tiff;*.tga;*.pcx;*.wbmp;*.j2k;*.jp2;;*.j2c;*.jpc;*.pgx;*.pnm;*.pgm;*.ppm;*.ras;*.mng;*.jng;*.ska;*.nef;*.crw;*.cr2;*.mrw;*.raf;*.erf;*.3fr;*.dcr;*.raw;*.dng;*.pef;*.x3f;*.arw;*.sr2;*.mef;*.orf|")
		_T("BMP Files(*.bmp)|*.bmp|")
		_T("GIF Files(*.gif)|*.gif|")
		_T("JPG Files(*.jpg;*.jpeg)|*.jpg;*.jpeg|")
		_T("PNG Files(*.png)|*.png|")
		_T("MNG Files(*.mng;*.jng;*.png)|*.mng;*.jng;*.png|")
		_T("ICO CUR Files(*.ico;*.cur)|*.ico;*.cur|")
		_T("TIF Files(*.tif;*.tiff)|*.tif;*.tiff|")
		_T("TGA Files(*.tga)|*.tga|")
		_T("PCX Files(*.pcx)|*.pcx|")
		_T("WBMP Files(*.wbmp)|*.wbmp|")
		_T("JP2 Files(*.j2k;*.jp2)|*.j2k;*.jp2|")
		_T("JPC Files(*.j2c;*.jpc)|*.j2c;*.jpc|")
		_T("PGX Files(*.pgx)|*.pgx|")
		_T("RAS Files(*.ras)|*.ras|")
		_T("PNM Files(*.pnm;*.pgm;*.ppm)|*.pnm;*.pgm;*.ppm|")
		_T("SKA Files(*.ska)|*.ska|")
		_T("RAW Files(*.nef;*.crw;*.cr2;*.mrw;*.raf;*.erf;*.3fr;*.dcr;*.raw;*.dng;*.pef;*.x3f;*.arw;*.sr2;*.mef;*.orf)|*.nef;*.crw;*.cr2;*.mrw;*.raf;*.erf;*.3fr;*.dcr;*.raw;*.dng;*.pef;*.x3f;*.arw;*.sr2;*.mef;*.orf|")
		_T("All Files(*.*)|*.*||");

	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);

	if(IDOK == fileDlg.DoModal())
	{
		OpenDocumentFile(fileDlg.GetPathName());
	}
}
