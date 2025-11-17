
// ImageProcessingDoc.h : CImageProcessingDoc 클래스의 인터페이스
//


#pragma once


class CImageProcessingDoc : public CDocument
{
protected: // serialization에서만 만들어집니다.
	CImageProcessingDoc();
	DECLARE_DYNCREATE(CImageProcessingDoc)

// 특성입니다.
public:
	inline CxImage *GetImage() { return m_pImage; }

private:
	CxImage * m_pImage;

// 작업입니다.
public:
	CString FindExtension(const CString& name);
	CString RemoveExtension(const CString& name);
	int FindType(const CString& ext);

// 재정의입니다.
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// 구현입니다.
public:
	virtual ~CImageProcessingDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	// 새 필터 핸들러 함수 선언
	public:
		// File Menu
		afx_msg void OnFileRevert();

		// Stylization Filters
		afx_msg void OnStylizeCartoon();
		afx_msg void OnStylizeOilPainting(); 
		afx_msg void OnStylizePopArt();
		afx_msg void OnStylizeEmboss();

		// Color & Mood Filters
		afx_msg void OnColorMoodRetro();
		afx_msg void OnColorMoodWarm();
		afx_msg void OnColorMoodCool();
		afx_msg void OnColorMoodBloom(); 
		afx_msg void OnColorMoodVignette();

	DECLARE_MESSAGE_MAP()	
};


