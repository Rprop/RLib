//
// DEELX Regular Expression Engine (v1.2)
//
// Copyright 2006 (c) RegExLab.com
// All Rights Reserved.
//
// http://www.regexlab.com/deelx/
//
// Author: 史寿伟 (sswater shi)
// sswater@gmail.com
//
// $Revision: 724 $
//

#ifndef __DEELX_REGEXP__H__
#define __DEELX_REGEXP__H__
#include "RLib_RegExp.h"

#pragma warning(disable:4244 4127 6308 6011 4505)

#include <memory.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

extern "C"
{
    typedef int(*POSIX_FUNC)(int);
    int isblank(int c);
}

//
// Data Reference
//
template <class R> class export RefBuffer
{
public:
    RefBuffer(const R *pcsz, int length);
    RefBuffer(const R *pcsz);

public:
    int nCompare(const R *pcsz)const;
    int nCompareNoCase(const R *pcsz)const;
    int Compare(const R *pcsz)const;
    int CompareNoCase(const R *pcsz)const;
    int Compare(const RefBuffer<R>  &)const;
    int CompareNoCase(const RefBuffer<R>  &)const;

    R At(int nIndex, R def = 0)const;
    R operator[](int nIndex)const;

    const R *GetBuffer()const;
    int GetSize()const;

public:
    virtual ~RefBuffer();

    // Content
protected:
    const R *m_pRef;
    int m_nSize;
};

#ifdef RLIB_DLL
//
// Implemenation
//
template <class R> RefBuffer<R>::RefBuffer(const R *pcsz, int length)
{
    m_pRef = pcsz;
    m_nSize = length;
}

//-------------------------------------------------------------------------

template <class R> RefBuffer<R>::RefBuffer(const R *pcsz)
{
    m_pRef = pcsz;
    m_nSize = 0;

    if (pcsz != 0)
        while (m_pRef[m_nSize] != 0)
        {
            m_nSize++;
        }
}

//-------------------------------------------------------------------------

template <class R> int RefBuffer<R>::nCompare(const R *pcsz)const
{
    for (int i = 0; i<m_nSize; i++)
    {
        if (m_pRef[i] != pcsz[i])
        {
            return m_pRef[i] - pcsz[i];
        }
    }

    return 0;
}

//-------------------------------------------------------------------------

template <class R> int RefBuffer<R>::nCompareNoCase(const R *pcsz)const
{
    for (int i = 0; i<m_nSize; i++)
    {
        if (m_pRef[i] != pcsz[i])
        {
            if (toupper((int)m_pRef[i]) != toupper((int)pcsz[i]))
            {
                return m_pRef[i] - pcsz[i];
            }
        }
    }

    return 0;
}

//-------------------------------------------------------------------------

template <class R> inline int RefBuffer<R>::Compare(const R *pcsz)const
{
    return nCompare(pcsz) ? 1 : (int)pcsz[m_nSize];
}

//-------------------------------------------------------------------------

template <class R> inline int RefBuffer<R>::CompareNoCase(const R *pcsz)const
{
    return nCompareNoCase(pcsz) ? 1 : (int)pcsz[m_nSize];
}

//-------------------------------------------------------------------------

template <class R> inline int RefBuffer<R>::Compare(const RefBuffer<R>  &cref)const
{
    return m_nSize == cref.m_nSize ? nCompare(cref.GetBuffer()): 1;
}

//-------------------------------------------------------------------------

template <class R> inline int RefBuffer<R>::CompareNoCase(const RefBuffer<R>  &cref)const
{
    return m_nSize == cref.m_nSize ? nCompareNoCase(cref.GetBuffer()): 1;
}

//-------------------------------------------------------------------------

template <class R> inline R RefBuffer<R>::At(int nIndex, R def)const
{
    return nIndex >= m_nSize ? def : m_pRef[nIndex];
}

//-------------------------------------------------------------------------

template <class R> inline R RefBuffer<R>::operator[](int nIndex)const
{
    return nIndex >= m_nSize ? 0 : m_pRef[nIndex];
}

//-------------------------------------------------------------------------

template <class R> const R *RefBuffer<R>::GetBuffer()const
{
    static const R _def[] = 
    {
        0
    };
    return m_pRef ? m_pRef : _def;
}

//-------------------------------------------------------------------------

template <class R> inline int RefBuffer<R>::GetSize()const
{
    return m_nSize;
}

//-------------------------------------------------------------------------

template <class R> RefBuffer<R>::~RefBuffer(){}
#endif // RLIB_DLL

//
// Data Buffer
//
template <class R> class export Buffer: public RefBuffer<R> 
{
public:
	RLIB_DECLARE_DYNCREATE;
    Buffer(const R *pcsz, int length);
    Buffer(const R *pcsz);
    Buffer();

public:
    R &operator[](int nIndex);
    const R &operator[](int nIndex)const;
    void Append(const R *pcsz, int length, int eol = 0);
    void Append(R el, int eol = 0);

public:
    void Push(R el);
    int Pop(R &el);
    int Peek(R &el)const;

public:
    const R *GetBuffer()const;
    R *GetBuffer();
    R *Detach();
    void Release();
    void Prepare(int index, int fill = 0);
    void Restore(int size);

public:
    virtual ~Buffer();

    // Content
protected:
    R *m_pBuffer;
    int m_nMaxLength;
};

#ifdef RLIB_DLL
//
// Implemenation
//
template <class R> Buffer<R>::Buffer(const R *pcsz, int length): RefBuffer<R> (0, length)
{
    m_nMaxLength = RefBuffer<R>::m_nSize + 1;

    RefBuffer<R>::m_pRef = m_pBuffer = (R*)RLIB_GlobalAlloc(sizeof(R) *m_nMaxLength);
    memcpy(m_pBuffer, pcsz, sizeof(R) *RefBuffer<R>::m_nSize);
    m_pBuffer[RefBuffer<R>::m_nSize] = 0;
}

//-------------------------------------------------------------------------

template <class R> Buffer<R>::Buffer(const R *pcsz): RefBuffer<R> (pcsz)
{
    m_nMaxLength = RefBuffer<R>::m_nSize + 1;

    RefBuffer<R>::m_pRef = m_pBuffer = (R*)RLIB_GlobalAlloc(sizeof(R) *m_nMaxLength);
    memcpy(m_pBuffer, pcsz, sizeof(R) *RefBuffer<R>::m_nSize);
    m_pBuffer[RefBuffer<R>::m_nSize] = 0;
}

//-------------------------------------------------------------------------

template <class R> Buffer<R>::Buffer(): RefBuffer<R> (0, 0)
{
    m_nMaxLength = 0;
    m_pBuffer = 0;
}

//-------------------------------------------------------------------------

template <class R> inline R &Buffer<R>::operator[](int nIndex)
{
    return m_pBuffer[nIndex];
}

//-------------------------------------------------------------------------

template <class R> inline const R &Buffer<R>::operator[](int nIndex)const
{
    return m_pBuffer[nIndex];
}

//-------------------------------------------------------------------------

template <class R> void Buffer<R>::Append(const R *pcsz, int length, int eol)
{
    int nNewLength = m_nMaxLength;

    // Check length
    if (nNewLength<8)
    {
        nNewLength = 8;
    }

    if (RefBuffer<R>::m_nSize + length + eol> nNewLength)
    {
        nNewLength *= 2;
    }

    if (RefBuffer<R>::m_nSize + length + eol> nNewLength)
    {
        nNewLength = RefBuffer<R>::m_nSize + length + eol + 11;
        nNewLength -= nNewLength % 8;
    }

    // Realloc
    if (nNewLength > m_nMaxLength)
    {
        R *new_pBuffer = (R*)AppBase::GetUsingPool()->ReAlloc(m_pBuffer, sizeof(R) *nNewLength);
        if (new_pBuffer != NULL)
        {
            RefBuffer < R > ::m_pRef = m_pBuffer = new_pBuffer;
            m_nMaxLength = nNewLength;
        } //if
    }

    // Append
    memcpy(m_pBuffer + RefBuffer < R > ::m_nSize, pcsz, sizeof(R) *length);
    RefBuffer < R > ::m_nSize += length;

    if (eol > 0)
    {
        m_pBuffer[RefBuffer < R > ::m_nSize] = 0;
    }
}

//-------------------------------------------------------------------------

template <class R> inline void Buffer<R>::Append(R el, int eol)
{
    Append(&el, 1, eol);
}

//-------------------------------------------------------------------------

template <class R> void Buffer<R>::Push(R el)
{
    // Realloc
    if (RefBuffer<R>::m_nSize >= m_nMaxLength)
    {
        int nNewLength = m_nMaxLength * 2;
        if (nNewLength<8)
        {
            nNewLength = 8;
        }

        R *new_pBuffer = (R*)AppBase::GetUsingPool()->ReAlloc(m_pBuffer, sizeof(R) *nNewLength);
        if (new_pBuffer != NULL)
        {
            RefBuffer<R>::m_pRef = m_pBuffer = new_pBuffer;
            m_nMaxLength = nNewLength;
        } //if
    }

    // Append
    m_pBuffer[RefBuffer < R > ::m_nSize++] = el;
}

//-------------------------------------------------------------------------

template <class R> inline int Buffer<R>::Pop(R &el)
{
    if (RefBuffer<R>::m_nSize> 0)
    {
        el = m_pBuffer[--RefBuffer<R>::m_nSize];
        return 1;
    }
    else
    {
        return 0;
    }
}

//-------------------------------------------------------------------------

template <class R> inline int Buffer<R>::Peek(R &el)const
{
    if (RefBuffer<R>::m_nSize> 0)
    {
        el = m_pBuffer[RefBuffer<R>::m_nSize - 1];
        return 1;
    }
    else
    {
        return 0;
    }
}

//-------------------------------------------------------------------------

template <class R> const R *Buffer<R>::GetBuffer()const
{
    static const R _def[] = 
    {
        0
    };
    return m_pBuffer ? m_pBuffer : _def;
}

//-------------------------------------------------------------------------

template <class R> R *Buffer<R>::GetBuffer()
{
    static const R _def[] = 
    {
        0
    };
    return m_pBuffer ? m_pBuffer : (R*)_def;
}

//-------------------------------------------------------------------------

template <class R> R *Buffer<R>::Detach()
{
    R *pBuffer = m_pBuffer;

    RefBuffer<R>::m_pRef = m_pBuffer = 0;
    RefBuffer<R>::m_nSize = m_nMaxLength = 0;

    return pBuffer;
}

//-------------------------------------------------------------------------

template <class R> void Buffer<R>::Release()
{
    R *pBuffer = Detach();

    if (pBuffer != 0)
    {
        AppBase::Collect(pBuffer);
    }
}

//-------------------------------------------------------------------------

template <class R> void Buffer<R>::Prepare(int index, int fill)
{
    int nNewSize = index + 1;

    // Realloc
    if (nNewSize> m_nMaxLength)
    {
        int nNewLength = m_nMaxLength;

        if (nNewLength<8)
        {
            nNewLength = 8;
        }

        if (nNewSize> nNewLength)
        {
            nNewLength *= 2;
        }

        if (nNewSize> nNewLength)
        {
            nNewLength = nNewSize + 11;
            nNewLength -= nNewLength % 8;
        }

        R *new_pBuffer = (R*)AppBase::GetUsingPool()->ReAlloc(m_pBuffer, sizeof(R) *nNewLength);
        if (new_pBuffer != NULL)
        {
            RefBuffer < R > ::m_pRef = m_pBuffer = new_pBuffer;
            m_nMaxLength = nNewLength;
        } //if
    }

    // size
    if (RefBuffer < R > ::m_nSize < nNewSize)
    {
        memset(m_pBuffer + RefBuffer < R > ::m_nSize, fill, sizeof(R)*(nNewSize - RefBuffer < R > ::m_nSize));
        RefBuffer < R > ::m_nSize = nNewSize;
    }
}

//-------------------------------------------------------------------------

template <class R> inline void Buffer<R>::Restore(int size)
{
    RefBuffer<R>::m_nSize = size;
}

//-------------------------------------------------------------------------

template <class R> Buffer<R>::~Buffer()
{
    if (m_pBuffer != 0)
    {
        AppBase::Collect(m_pBuffer);
    }
}

//-------------------------------------------------------------------------

#endif // RLIB_DLL

/// <summary>
/// 存放匹配过程中的数据
/// 当需要在同一段文本中多次连续匹配时，对同一个 CContext 对象多次匹配，可以节约系统资源
/// </summary>
class CContext
{
public:
    Buffer < int > m_stack;
    Buffer < int > m_capturestack, m_captureindex;

public:
    int m_nCurrentPos;
    int m_nBeginPos;
    int m_nLastBeginPos;
    int m_nParenZindex;

    void *m_pMatchString;
    int m_pMatchStringLength;
};

//
// Interface
//
class ElxInterface
{
public:
    virtual int Match(CContext *pContext)const = 0;
    virtual int MatchNext(CContext *pContext)const = 0;

public:
    virtual ~ElxInterface(){}
    ;
};

//
// Alternative
//
template <int x> class CAlternativeElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CAlternativeElxT();

public:
    Buffer<ElxInterface *> m_elxlist;
};

typedef CAlternativeElxT < 0 > CAlternativeElx;

//
// Assert
//
template <int x> class CAssertElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CAssertElxT(ElxInterface *pelx, int byes = 1);

public:
    ElxInterface *m_pelx;
    int m_byes;
};

typedef CAssertElxT < 0 > CAssertElx;

//
// Back reference elx
//
template <class CHART> class CBackrefElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CBackrefElxT(int nnumber, int brightleft, int bignorecase);

public:
    int m_nnumber;
    int m_brightleft;
    int m_bignorecase;

    Buffer<CHART> m_szNamed;
};

//
// Implementation
//
template <class CHART> CBackrefElxT<CHART>::CBackrefElxT(int nnumber, int brightleft, int bignorecase)
{
    m_nnumber = nnumber;
    m_brightleft = brightleft;
    m_bignorecase = bignorecase;
}

//-------------------------------------------------------------------------

template <class CHART> int CBackrefElxT<CHART>::Match(CContext *pContext)const
{
    // check number, for named
    if (m_nnumber<0 || m_nnumber >= pContext->m_captureindex.GetSize())
    {
        return 0;
    }

    int index = pContext->m_captureindex[m_nnumber];
    if (index<0)
    {
        return 0;
    }

    // check enclosed
    int pos1 = pContext->m_capturestack[index + 1];
    int pos2 = pContext->m_capturestack[index + 2];

    if (pos2<0)
    {
        pos2 = pContext->m_nCurrentPos;
    }

    // info
    int lpos = pos1<pos2 ? pos1 : pos2;
    int rpos = pos1<pos2 ? pos2 : pos1;
    int slen = rpos - lpos;

    const CHART *pcsz = (const CHART*)pContext->m_pMatchString;
    int npos = pContext->m_nCurrentPos;
    int tlen = pContext->m_pMatchStringLength;

    // compare
    int bsucc;
    RefBuffer<CHART> refstr(pcsz + lpos, slen);

    if (m_brightleft)
    {
        if (npos<slen)
        {
            return 0;
        }

        if (m_bignorecase)
        {
            bsucc = !refstr.nCompareNoCase(pcsz + (npos - slen));
        }
        else
        {
            bsucc = !refstr.nCompare(pcsz + (npos - slen));
        }

        if (bsucc)
        {
            pContext->m_stack.Push(npos);
            pContext->m_nCurrentPos -= slen;
        }
    }
    else
    {
        if (npos + slen > tlen)
        {
            return 0;
        }

        if (m_bignorecase)
        {
            bsucc = !refstr.nCompareNoCase(pcsz + npos);
        }
        else
        {
            bsucc = !refstr.nCompare(pcsz + npos);
        }

        if (bsucc)
        {
            pContext->m_stack.Push(npos);
            pContext->m_nCurrentPos += slen;
        }
    }

    return bsucc;
}

//-------------------------------------------------------------------------

template <class CHART> int CBackrefElxT<CHART>::MatchNext(CContext *pContext)const
{
    int npos = 0;

    pContext->m_stack.Pop(npos);
    pContext->m_nCurrentPos = npos;

    return 0;
}

// RCHART
#ifndef RCHART
#define RCHART T
#endif 

// BOUNDARY_TYPE
enum BOUNDARY_TYPE
{
    BOUNDARY_FILE_BEGIN,  // begin of whole text
    BOUNDARY_FILE_END,  // end of whole text
    BOUNDARY_FILE_END_N,  // end of whole text, or before newline at the end
    BOUNDARY_LINE_BEGIN,  // begin of line
    BOUNDARY_LINE_END,  // end of line
    BOUNDARY_WORD_BEGIN,  // begin of word
    BOUNDARY_WORD_END,  // end of word
    BOUNDARY_WORD_EDGE
};

//
// Boundary Elx
//
template <class CHART> class CBoundaryElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CBoundaryElxT(int ntype, int byes = 1);

protected:
    static int IsWordChar(CHART ch);

public:
    int m_ntype;
    int m_byes;
};

//
// Implementation
//
template <class CHART> CBoundaryElxT<CHART>::CBoundaryElxT(int ntype, int byes)
{
    m_ntype = ntype;
    m_byes = byes;
}

//-------------------------------------------------------------------------

template <class CHART> int CBoundaryElxT<CHART>::Match(CContext *pContext)const
{
    const CHART *pcsz = (const CHART*)pContext->m_pMatchString;
    int npos = pContext->m_nCurrentPos;
    int tlen = pContext->m_pMatchStringLength;

    CHART chL = npos> 0 ? pcsz[npos - 1]: 0;
    CHART chR = npos<tlen ? pcsz[npos]: 0;

    int bsucc = 0;

    switch (m_ntype)
    {
        case BOUNDARY_FILE_BEGIN:
            bsucc = (npos <= 0);
            break;

        case BOUNDARY_FILE_END:
            bsucc = (npos >= tlen);
            break;

        case BOUNDARY_FILE_END_N:
            bsucc = (npos >= tlen) || (pcsz[tlen - 1] == RCHART('\n') && (npos == tlen - 1 || (pcsz[tlen - 2] == RCHART('\r') && npos == tlen - 2)));
            break;

        case BOUNDARY_LINE_BEGIN:
            bsucc = (npos <= 0) || (chL == RCHART('\n')) || ((chL == RCHART('\r')) && (chR != RCHART('\n')));
            break;

        case BOUNDARY_LINE_END:
            bsucc = (npos >= tlen) || (chR == RCHART('\r')) || ((chR == RCHART('\n')) && (chL != RCHART('\r')));
            break;

        case BOUNDARY_WORD_BEGIN:
            bsucc = !IsWordChar(chL) && IsWordChar(chR);
            break;

        case BOUNDARY_WORD_END:
            bsucc = IsWordChar(chL) && !IsWordChar(chR);
            break;

        case BOUNDARY_WORD_EDGE:
            bsucc = IsWordChar(chL) ? !IsWordChar(chR): IsWordChar(chR);
            break;
    }

    return m_byes ? bsucc : !bsucc;
}

//-------------------------------------------------------------------------

template <class CHART> int CBoundaryElxT<CHART>::MatchNext(CContext*)const
{
    return 0;
}

//-------------------------------------------------------------------------

template <class CHART> inline int CBoundaryElxT<CHART>::IsWordChar(CHART ch)
{
    return (ch >= RCHART('A') && ch <= RCHART('Z')) || (ch >= RCHART('a') && ch <= RCHART('z')) || (ch >= RCHART('0') && ch <= RCHART('9')) || (ch == RCHART('_'));
}

//
// Bracket
//
template <class CHART> class CBracketElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CBracketElxT(int nnumber, int bright);
    int CheckCaptureIndex(int &index, CContext *pContext)const;

public:
    int m_nnumber;
    int m_bright;

    Buffer<CHART> m_szNamed;
};

template <class CHART> CBracketElxT<CHART>::CBracketElxT(int nnumber, int bright)
{
    m_nnumber = nnumber;
    m_bright = bright;
}

//-------------------------------------------------------------------------

template <class CHART> inline int CBracketElxT<CHART>::CheckCaptureIndex(int &index, CContext *pContext)const
{
    if (index >= pContext->m_capturestack.GetSize())
    {
        index = pContext->m_capturestack.GetSize() - 4;
    }

    while (index >= 0)
    {
        if (pContext->m_capturestack[index] == m_nnumber)
        {
            return 1;
        }

        index -= 4;
    }


    return 0;
}

//
// capturestack[index+0] => Group number
// capturestack[index+1] => Capture start pos
// capturestack[index+2] => Capture end pos
// capturestack[index+3] => Capture enclose z-index, zindex<0 means inner group with same name
//
template <class CHART> int CBracketElxT<CHART>::Match(CContext *pContext)const
{
    // check, for named
    if (m_nnumber<0)
    {
        return 0;
    }

    if (!m_bright)
    {
        pContext->m_captureindex.Prepare(m_nnumber,  - 1);
        int index = pContext->m_captureindex[m_nnumber];

        // check
        if (CheckCaptureIndex(index, pContext) && pContext->m_capturestack[index + 2]<0)
        {
            pContext->m_capturestack[index + 3]--;
            return 1;
        }

        // save
        pContext->m_captureindex[m_nnumber] = pContext->m_capturestack.GetSize();

        pContext->m_capturestack.Push(m_nnumber);
        pContext->m_capturestack.Push(pContext->m_nCurrentPos);
        pContext->m_capturestack.Push( - 1);
        pContext->m_capturestack.Push(0); // z-index
    }
    else
    {
        // check
        int index = pContext->m_captureindex[m_nnumber];

        if (CheckCaptureIndex(index, pContext))
        {
            if (pContext->m_capturestack[index + 3] < 0)
            // check inner group with same name
            {
                pContext->m_capturestack[index + 3]++;
                return 1;
            }

            // save
            pContext->m_capturestack[index + 2] = pContext->m_nCurrentPos;
            pContext->m_capturestack[index + 3] = pContext->m_nParenZindex++;
        }
    }

    return 1;
}

//-------------------------------------------------------------------------

template <class CHART> int CBracketElxT<CHART>::MatchNext(CContext *pContext)const
{
    int index = pContext->m_captureindex[m_nnumber];
    if (!CheckCaptureIndex(index, pContext))
    {
        return 0;
    }

    if (!m_bright)
    {
        if (pContext->m_capturestack[index + 3] < 0)
        {
            pContext->m_capturestack[index + 3]++;
            return 0;
        }

        pContext->m_capturestack.Restore(pContext->m_capturestack.GetSize() - 4);

        // to find
        CheckCaptureIndex(index, pContext);

        // new index
        pContext->m_captureindex[m_nnumber] = index;
    }
    else
    {
        if (pContext->m_capturestack[index + 2] >= 0)
        {
            pContext->m_capturestack[index + 2] =  - 1;
            pContext->m_capturestack[index + 3] = 0;
        }
        else
        {
            pContext->m_capturestack[index + 3]--;
        }
    }

    return 0;
}

//
// Deletage
//
template <class CHART> class CDelegateElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CDelegateElxT(int ndata = 0);

public:
    ElxInterface *m_pelx;
    int m_ndata; // +0 : recursive to
    // -3 : named recursive

    Buffer<CHART> m_szNamed;
};

template <class CHART> CDelegateElxT<CHART>::CDelegateElxT(int ndata)
{
    m_pelx = 0;
    m_ndata = ndata;
}

//-------------------------------------------------------------------------

template <class CHART> int CDelegateElxT<CHART>::Match(CContext *pContext)const
{
    if (m_pelx != 0)
    {
        return m_pelx->Match(pContext);
    }
    else
    {
        return 1;
    }
}

//-------------------------------------------------------------------------

template <class CHART> int CDelegateElxT<CHART>::MatchNext(CContext *pContext)const
{
    if (m_pelx != 0)
    {
        return m_pelx->MatchNext(pContext);
    }
    else
    {
        return 0;
    }
}

//
// Empty
//
template <int x> class CEmptyElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CEmptyElxT();
};

typedef CEmptyElxT < 0 > CEmptyElx;

//
// Global
//
template <int x> class CGlobalElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CGlobalElxT();
};

typedef CGlobalElxT < 0 > CGlobalElx;

//
// Repeat
//
template <int x> class CRepeatElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CRepeatElxT(ElxInterface *pelx, int ntimes);

protected:
    int MatchFixed(CContext *pContext)const;
    int MatchNextFixed(CContext *pContext)const;

public:
    ElxInterface *m_pelx;
    int m_nfixed;
};

typedef CRepeatElxT < 0 > CRepeatElx;

//
// Greedy
//
template <int x> class CGreedyElxT: public CRepeatElxT<x> 
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CGreedyElxT(ElxInterface *pelx, int nmin = 0, int nmax = INT_MAX);

protected:
    int MatchVart(CContext *pContext)const;
    int MatchNextVart(CContext *pContext)const;

public:
    int m_nvart;
};

typedef CGreedyElxT < 0 > CGreedyElx;

//
// Independent
//
template <int x> class CIndependentElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CIndependentElxT(ElxInterface *pelx);

public:
    ElxInterface *m_pelx;
};

typedef CIndependentElxT < 0 > CIndependentElx;

//
// List
//
template <int x> class CListElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CListElxT(int brightleft);

public:
    Buffer<ElxInterface *> m_elxlist;
    int m_brightleft;
};

typedef CListElxT < 0 > CListElx;

//
// Posix Elx
//
template <class CHART> class CPosixElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CPosixElxT(const char *posix, int brightleft);

public:
    POSIX_FUNC m_posixfun;
    int m_brightleft;
    int m_byes;
};

//
// Implementation
//
template <class CHART> CPosixElxT<CHART>::CPosixElxT(const char *posix, int brightleft)
{
    m_brightleft = brightleft;

    if (posix[1] == '^')
    {
        m_byes = 0;
        posix += 2;
    }
    else
    {
        m_byes = 1;
        posix += 1;
    }
    if (!strncmp(posix, "alnum:", 6))
    {
        m_posixfun = ::isalnum;
    }
    else if (!strncmp(posix, "alpha:", 6))
    {
        m_posixfun = ::isalpha;
    }
    else if (!strncmp(posix, "ascii:", 6))
    {
        m_posixfun = ::isascii;
    }
    else if (!strncmp(posix, "cntrl:", 6))
    {
        m_posixfun = ::iscntrl;
    }
    else if (!strncmp(posix, "digit:", 6))
    {
        m_posixfun = ::isdigit;
    }
    else if (!strncmp(posix, "graph:", 6))
    {
        m_posixfun = ::isgraph;
    }
    else if (!strncmp(posix, "lower:", 6))
    {
        m_posixfun = ::islower;
    }
    else if (!strncmp(posix, "print:", 6))
    {
        m_posixfun = ::isprint;
    }
    else if (!strncmp(posix, "punct:", 6))
    {
        m_posixfun = ::ispunct;
    }
    else if (!strncmp(posix, "space:", 6))
    {
        m_posixfun = ::isspace;
    }
    else if (!strncmp(posix, "upper:", 6))
    {
        m_posixfun = ::isupper;
    }
    else if (!strncmp(posix, "xdigit:", 7))
    {
        m_posixfun = (POSIX_FUNC)::isxdigit;
    }
    else if (!strncmp(posix, "blank:", 6))
    {
        m_posixfun = isblank;
    }
    else
    {
        m_posixfun = 0;
    }
}

//-------------------------------------------------------------------------

inline int isblank(int c)
{
    return c == 0x20 || c == '\t';
}

//-------------------------------------------------------------------------

template <class CHART> int CPosixElxT<CHART>::Match(CContext *pContext)const
{
    if (m_posixfun == 0)
    {
        return 0;
    }

    int tlen = pContext->m_pMatchStringLength;
    int npos = pContext->m_nCurrentPos;

    // check
    int at = m_brightleft ? npos - 1: npos;
    if (at<0 || at >= tlen)
    {
        return 0;
    }

    CHART ch = ((const CHART*)pContext->m_pMatchString)[at];

    int bsucc = (*m_posixfun)(ch);

    if (!m_byes)
    {
        bsucc = !bsucc;
    }

    if (bsucc)
    {
        pContext->m_nCurrentPos += m_brightleft ?  - 1: 1;
    }

    return bsucc;
}

//-------------------------------------------------------------------------

template <class CHART> int CPosixElxT<CHART>::MatchNext(CContext *pContext)const
{
    pContext->m_nCurrentPos -= m_brightleft ?  - 1: 1;
    return 0;
}

//
// Possessive
//
template <int x> class CPossessiveElxT: public CGreedyElxT<x> 
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CPossessiveElxT(ElxInterface *pelx, int nmin = 0, int nmax = INT_MAX);
};

typedef CPossessiveElxT < 0 > CPossessiveElx;

//
// Range Elx
//
template <class CHART> class CRangeElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CRangeElxT(int brightleft, int byes);

public:
    int IsContainChar(CHART ch)const;

public:
    Buffer<CHART> m_ranges;
    Buffer<CHART> m_chars;
    Buffer<ElxInterface *> m_embeds;

public:
    int m_brightleft;
    int m_byes;
};

//
// Implementation
//
template <class CHART> CRangeElxT<CHART>::CRangeElxT(int brightleft, int byes)
{
    m_brightleft = brightleft;
    m_byes = byes;
}

//-------------------------------------------------------------------------

template <class CHART> int CRangeElxT<CHART>::Match(CContext *pContext)const
{
    int tlen = pContext->m_pMatchStringLength;
    int npos = pContext->m_nCurrentPos;

    // check
    int at = m_brightleft ? npos - 1: npos;
    if (at<0 || at >= tlen)
    {
        return 0;
    }

    CHART ch = ((const CHART*)pContext->m_pMatchString)[at];
    int bsucc = 0, i;

    // compare
    for (i = 0; !bsucc && i<m_ranges.GetSize(); i += 2)
    {
        if (m_ranges[i] <= ch && ch <= m_ranges[i + 1])
        {
            bsucc = 1;
        }
    }

    for (i = 0; !bsucc && i < m_chars.GetSize(); i++)
    {
        if (m_chars[i] == ch)
        {
            bsucc = 1;
        }
    }

    for (i = 0; !bsucc && i < m_embeds.GetSize(); i++)
    {
        if (m_embeds[i]->Match(pContext))
        {
            pContext->m_nCurrentPos = npos;
            bsucc = 1;
        }
    }

    if (!m_byes)
    {
        bsucc = !bsucc;
    }

    if (bsucc)
    {
        pContext->m_nCurrentPos += m_brightleft ?  - 1: 1;
    }

    return bsucc;
}

//-------------------------------------------------------------------------

template <class CHART> int CRangeElxT<CHART>::IsContainChar(CHART ch)const
{
    int bsucc = 0, i;

    // compare
    for (i = 0; !bsucc && i<m_ranges.GetSize(); i += 2)
    {
        if (m_ranges[i] <= ch && ch <= m_ranges[i + 1])
        {
            bsucc = 1;
        }
    }

    for (i = 0; !bsucc && i < m_chars.GetSize(); i++)
    {
        if (m_chars[i] == ch)
        {
            bsucc = 1;
        }
    }

    return bsucc;
}

//-------------------------------------------------------------------------

template <class CHART> int CRangeElxT<CHART>::MatchNext(CContext *pContext)const
{
    pContext->m_nCurrentPos -= m_brightleft ?  - 1: 1;
    return 0;
}

//
// Reluctant
//
template <int x> class CReluctantElxT: public CRepeatElxT<x> 
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CReluctantElxT(ElxInterface *pelx, int nmin = 0, int nmax = INT_MAX);

protected:
    int MatchVart(CContext *pContext)const;
    int MatchNextVart(CContext *pContext)const;

public:
    int m_nvart;
};

typedef CReluctantElxT < 0 > CReluctantElx;

//
// String Elx
//
template <class CHART> class CStringElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CStringElxT(const CHART *fixed, int nlength, int brightleft, int bignorecase);

public:
    Buffer<CHART> m_szPattern;
    int m_brightleft;
    int m_bignorecase;
};

//
// Implementation
//
template <class CHART> CStringElxT<CHART>::CStringElxT(const CHART *fixed, int nlength, int brightleft, int bignorecase): m_szPattern(fixed, nlength)
{
    m_brightleft = brightleft;
    m_bignorecase = bignorecase;
}

//-------------------------------------------------------------------------

template <class CHART> int CStringElxT<CHART>::Match(CContext *pContext)const
{
    const CHART *pcsz = (const CHART*)pContext->m_pMatchString;
    int npos = pContext->m_nCurrentPos;
    int tlen = pContext->m_pMatchStringLength;
    int slen = m_szPattern.GetSize();

    int bsucc;

    if (m_brightleft)
    {
        if (npos<slen)
        {
            return 0;
        }

        if (m_bignorecase)
        {
            bsucc = !m_szPattern.nCompareNoCase(pcsz + (npos - slen));
        }
        else
        {
            bsucc = !m_szPattern.nCompare(pcsz + (npos - slen));
        }

        if (bsucc)
        {
            pContext->m_nCurrentPos -= slen;
        }
    }
    else
    {
        if (npos + slen > tlen)
        {
            return 0;
        }

        if (m_bignorecase)
        {
            bsucc = !m_szPattern.nCompareNoCase(pcsz + npos);
        }
        else
        {
            bsucc = !m_szPattern.nCompare(pcsz + npos);
        }

        if (bsucc)
        {
            pContext->m_nCurrentPos += slen;
        }
    }

    return bsucc;
}

//-------------------------------------------------------------------------

template <class CHART> int CStringElxT<CHART>::MatchNext(CContext *pContext)const
{
    int slen = m_szPattern.GetSize();

    if (m_brightleft)
    {
        pContext->m_nCurrentPos += slen;
    }
    else
    {
        pContext->m_nCurrentPos -= slen;
    }

    return 0;
}

//
// CConditionElx
//
template <class CHART> class CConditionElxT: public ElxInterface
{
public:
    int Match(CContext *pContext)const;
    int MatchNext(CContext *pContext)const;

public:
    CConditionElxT();

public:
    // backref condition
    int m_nnumber;
    Buffer<CHART> m_szNamed;

    // elx condition
    ElxInterface *m_pelxask;

    // selection
    ElxInterface *m_pelxyes,  *m_pelxno;
};

template <class CHART> CConditionElxT<CHART>::CConditionElxT()
{
    m_nnumber =  - 1;
}

//-------------------------------------------------------------------------

template <class CHART> int CConditionElxT<CHART>::Match(CContext *pContext)const
{
    // status
    int nbegin = pContext->m_nCurrentPos;
    int nsize = pContext->m_stack.GetSize();
    int ncsize = pContext->m_capturestack.GetSize();

    // condition result
    int condition_yes = 0;

    // backref type
    if (m_nnumber >= 0)
    {
        do
        {
            if (m_nnumber >= pContext->m_captureindex.GetSize())
            {
                break;
            }

            int index = pContext->m_captureindex[m_nnumber];
            if (index<0)
            {
                break;
            }

            // else valid
            condition_yes = 1;
        }
        while (0);
    }
    else
    {
        if (m_pelxask == 0)
        {
            condition_yes = 1;
        }
        else
        {
            condition_yes = m_pelxask->Match(pContext);
        }

        pContext->m_stack.Restore(nsize);
        pContext->m_nCurrentPos = nbegin;
    }

    // elx result
    int bsucc;
    if (condition_yes)
    {
        bsucc = m_pelxyes == 0 ? 1 : m_pelxyes->Match(pContext);
    }
    else
    {
        bsucc = m_pelxno == 0 ? 1 : m_pelxno->Match(pContext);
    }

    if (bsucc)
    {
        pContext->m_stack.Push(ncsize);
        pContext->m_stack.Push(condition_yes);
    }
    else
    {
        pContext->m_capturestack.Restore(ncsize);
    }

    return bsucc;
}

//-------------------------------------------------------------------------

template <class CHART> int CConditionElxT<CHART>::MatchNext(CContext *pContext)const
{
    // pop
    int ncsize, condition_yes;

    pContext->m_stack.Pop(condition_yes);
    pContext->m_stack.Pop(ncsize);

    // elx result
    int bsucc;
    if (condition_yes)
    {
        bsucc = m_pelxyes == 0 ? 0 : m_pelxyes->MatchNext(pContext);
    }
    else
    {
        bsucc = m_pelxno == 0 ? 0 : m_pelxno->MatchNext(pContext);
    }

    if (bsucc)
    {
        pContext->m_stack.Push(ncsize);
        pContext->m_stack.Push(condition_yes);
    }
    else
    {
        pContext->m_capturestack.Restore(ncsize);
    }

    return bsucc;
}

//
// MatchResult
//
template <int x> class MatchResultT
{
public:
    /// <summary>
    /// 获取是否匹配成功
    /// </summary>
    /// <returns>返回非零值表示匹配成功, 返回 0 表示匹配失败</returns>
    int IsMatched()const;

    /// <summary>
    /// 获取是否匹配成功
    /// </summary>
    /// <returns>返回非零值表示匹配成功, 返回 0 表示匹配失败</returns>
    __declspec(property(get = IsMatched))const int Succeed;

public:
    /// <summary>
    /// 获取匹配到的子字符串开始位置
    /// </summary>
    /// <returns>匹配成功后, 获取所匹配到的子字符串的开始位置. 如果匹配失败, 则返回负值</returns>
    int GetStart()const;
    /// <summary>
    /// 获取匹配到的子字符串结束位置
    /// </summary>
    /// <returns>匹配成功后, 获取所匹配到的子字符串的结束位置. 如果匹配失败, 则返回负值</returns>
    int GetEnd()const;

public:
    /// <summary>
    /// 获取正则表达式最大捕获组编号
    /// </summary>
    /// <returns>返回最大分组编号</returns>
    int MaxGroupNumber()const;
    /// <summary>
    /// 获取指定分组捕获的字符串的开始位置
    /// </summary>
    /// <param name="nGroupNumber">分组编号</param>
    /// <returns>返回指定分组捕获的字符串的开始位置. 如果指定分组未捕获, 则返回负值</returns>
    int GetGroupStart(int nGroupNumber)const;
    /// <summary>
    /// 获取指定分组捕获的字符串的结束位置
    /// </summary>
    /// <param name="nGroupNumber">分组编号</param>
    /// <returns>返回指定分组捕获的字符串的结束位置. 如果指定分组未捕获, 则返回负值</returns>
    int GetGroupEnd(int nGroupNumber)const;

public:
    MatchResultT(const MatchResultT<x>  &from)
    {
         *this = from;
    }
    MatchResultT(CContext *pContext = 0, int nMaxNumber =  - 1);
    MatchResultT < x >  &operator = (const MatchResultT < x >  &);
    inline operator int()const
    {
        return IsMatched();
    }

public:
    Buffer < int > m_result;
};

// Stocked Elx IDs
enum STOCKELX_ID_DEFINES
{
    STOCKELX_EMPTY = 0, 

    ///////////////////////

    STOCKELX_DOT_ALL, STOCKELX_DOT_NOT_ALL, 

    STOCKELX_WORD, STOCKELX_WORD_NOT, 

    STOCKELX_SPACE, STOCKELX_SPACE_NOT, 

    STOCKELX_DIGITAL, STOCKELX_DIGITAL_NOT, 

    //////////////////////

    STOCKELX_DOT_ALL_RIGHTLEFT, STOCKELX_DOT_NOT_ALL_RIGHTLEFT, 

    STOCKELX_WORD_RIGHTLEFT, STOCKELX_WORD_RIGHTLEFT_NOT, 

    STOCKELX_SPACE_RIGHTLEFT, STOCKELX_SPACE_RIGHTLEFT_NOT, 

    STOCKELX_DIGITAL_RIGHTLEFT, STOCKELX_DIGITAL_RIGHTLEFT_NOT, 

    /////////////////////

    STOCKELX_COUNT
};

// REGEX_FLAGS
#ifndef _REGEX_FLAGS_DEFINED
enum REGEX_FLAGS
{
    NO_FLAG = 0, SINGLELINE = 0x01, MULTILINE = 0x02, GLOBAL = 0x04, IGNORECASE = 0x08, RIGHTTOLEFT = 0x10, EXTENDED = 0x20
};
#define _REGEX_FLAGS_DEFINED
#endif 

//
// All implementations
//
template <int x> CAlternativeElxT<x>::CAlternativeElxT(){}

template <int x> int CAlternativeElxT<x>::Match(CContext *pContext)const
{
    if (m_elxlist.GetSize() == 0)
    {
        return 1;
    }

    // try all
    for (int n = 0; n<m_elxlist.GetSize(); n++)
    {
        if (m_elxlist[n]->Match(pContext))
        {
            pContext->m_stack.Push(n);
            return 1;
        }
    }

    return 0;
}

//-------------------------------------------------------------------------

template <int x> int CAlternativeElxT<x>::MatchNext(CContext *pContext)const
{
    if (m_elxlist.GetSize() == 0)
    {
        return 0;
    }

    int n = 0;

    // recall prev
    pContext->m_stack.Pop(n);

    // prev
    if (m_elxlist[n]->MatchNext(pContext))
    {
        pContext->m_stack.Push(n);
        return 1;
    }
    else
    {
        // try rest
        for (n++; n < m_elxlist.GetSize(); n++)
        {
            if (m_elxlist[n]->Match(pContext))
            {
                pContext->m_stack.Push(n);
                return 1;
            }
        }

        return 0;
    }
}

// assertx.cpp: implementation of the CAssertElx class.
//
template <int x> CAssertElxT<x>::CAssertElxT(ElxInterface *pelx, int byes)
{
    m_pelx = pelx;
    m_byes = byes;
}

//-------------------------------------------------------------------------

template <int x> int CAssertElxT<x>::Match(CContext *pContext)const
{
    int nbegin = pContext->m_nCurrentPos;
    int nsize = pContext->m_stack.GetSize();
    int ncsize = pContext->m_capturestack.GetSize();
    int bsucc;

    // match
    if (m_byes)
    {
        bsucc = m_pelx->Match(pContext);
    }
    else
    {
        bsucc = !m_pelx->Match(pContext);
    }

    // status
    pContext->m_stack.Restore(nsize);
    pContext->m_nCurrentPos = nbegin;

    if (bsucc)
    {
        pContext->m_stack.Push(ncsize);
    }
    else
    {
        pContext->m_capturestack.Restore(ncsize);
    }

    return bsucc;
}

//-------------------------------------------------------------------------

template <int x> int CAssertElxT<x>::MatchNext(CContext *pContext)const
{
    int ncsize = 0;

    pContext->m_stack.Pop(ncsize);
    pContext->m_capturestack.Restore(ncsize);

    return 0;
}

// emptyelx.cpp: implementation of the CEmptyElx class.
//
template <int x> CEmptyElxT<x>::CEmptyElxT(){}

template <int x> int CEmptyElxT<x>::Match(CContext*)const
{
    return 1;
}

//-------------------------------------------------------------------------

template <int x> int CEmptyElxT<x>::MatchNext(CContext*)const
{
    return 0;
}

// globalx.cpp: implementation of the CGlobalElx class.
//
template <int x> CGlobalElxT<x>::CGlobalElxT(){}

template <int x> int CGlobalElxT<x>::Match(CContext *pContext)const
{
    return pContext->m_nCurrentPos == pContext->m_nBeginPos;
}

//-------------------------------------------------------------------------

template <int x> int CGlobalElxT<x>::MatchNext(CContext*)const
{
    return 0;
}

// greedelx.cpp: implementation of the CGreedyElx class.
//
template <int x> CGreedyElxT<x>::CGreedyElxT(ElxInterface *pelx, int nmin, int nmax): CRepeatElxT<x> (pelx, nmin)
{
    m_nvart = nmax - nmin;
}

//-------------------------------------------------------------------------

template <int x> int CGreedyElxT<x>::Match(CContext *pContext)const
{
    if (!CRepeatElxT<x>::MatchFixed(pContext))
    {
        return 0;
    }

    while (!MatchVart(pContext))
    {
        if (!CRepeatElxT<x>::MatchNextFixed(pContext))
        {
            return 0;
        }
    }

    return 1;
}

//-------------------------------------------------------------------------

template <int x> int CGreedyElxT<x>::MatchNext(CContext *pContext)const
{
    if (MatchNextVart(pContext))
    {
        return 1;
    }

    if (!CRepeatElxT<x>::MatchNextFixed(pContext))
    {
        return 0;
    }

    while (!MatchVart(pContext))
    {
        if (!CRepeatElxT<x>::MatchNextFixed(pContext))
        {
            return 0;
        }
    }

    return 1;
}

//-------------------------------------------------------------------------

template <int x> int CGreedyElxT<x>::MatchVart(CContext *pContext)const
{
    int n = 0;
    int nbegin = pContext->m_nCurrentPos;

    while (n<m_nvart && CRepeatElxT<x>::m_pelx->Match(pContext))
    {
        while (pContext->m_nCurrentPos == nbegin)
        {
            if (!CRepeatElxT<x>::m_pelx->MatchNext(pContext))
            {
                break;
            }
        }

        if (pContext->m_nCurrentPos == nbegin)
        {
            break;
        }

        n++;
        nbegin = pContext->m_nCurrentPos;
    }

    pContext->m_stack.Push(n);

    return 1;
}

//-------------------------------------------------------------------------

template <int x> int CGreedyElxT<x>::MatchNextVart(CContext *pContext)const
{
    int n = 0;
    pContext->m_stack.Pop(n);

    if (n == 0)
    {
        return 0;
    }

    if (!CRepeatElxT<x>::m_pelx->MatchNext(pContext))
    {
        n--;
    }

    pContext->m_stack.Push(n);

    return 1;
}

// indepelx.cpp: implementation of the CIndependentElx class.
//
template <int x> CIndependentElxT<x>::CIndependentElxT(ElxInterface *pelx)
{
    m_pelx = pelx;
}

//-------------------------------------------------------------------------

template <int x> int CIndependentElxT<x>::Match(CContext *pContext)const
{
    int nbegin = pContext->m_nCurrentPos;
    int nsize = pContext->m_stack.GetSize();
    int ncsize = pContext->m_capturestack.GetSize();

    // match
    int bsucc = m_pelx->Match(pContext);

    // status
    pContext->m_stack.Restore(nsize);

    if (bsucc)
    {
        pContext->m_stack.Push(nbegin);
        pContext->m_stack.Push(ncsize);
    }

    return bsucc;
}

//-------------------------------------------------------------------------

template <int x> int CIndependentElxT<x>::MatchNext(CContext *pContext)const
{
    int nbegin = 0, ncsize = 0;

    pContext->m_stack.Pop(ncsize);
    pContext->m_stack.Pop(nbegin);

    pContext->m_capturestack.Restore(ncsize);
    pContext->m_nCurrentPos = nbegin;

    return 0;
}

// listelx.cpp: implementation of the CListElx class.
//
template <int x> CListElxT<x>::CListElxT(int brightleft)
{
    m_brightleft = brightleft;
}

//-------------------------------------------------------------------------

template <int x> int CListElxT<x>::Match(CContext *pContext)const
{
    if (m_elxlist.GetSize() == 0)
    {
        return 1;
    }

    // prepare
    int bol = m_brightleft ? m_elxlist.GetSize():  - 1;
    int stp = m_brightleft ?  - 1: 1;
    int eol = m_brightleft ?  - 1: m_elxlist.GetSize();

    // from first
    int n = bol + stp;

    // match all
    while (n != eol)
    {
        if (m_elxlist[n]->Match(pContext))
        {
            n += stp;
        }
        else
        {
            n -= stp;

            while (n != bol && !m_elxlist[n]->MatchNext(pContext))
            {
                n -= stp;
            }

            if (n != bol)
            {
                n += stp;
            }
            else
            {
                return 0;
            }
        }
    }

    return 1;
}

//-------------------------------------------------------------------------

template <int x> int CListElxT<x>::MatchNext(CContext *pContext)const
{
    if (m_elxlist.GetSize() == 0)
    {
        return 0;
    }

    // prepare
    int bol = m_brightleft ? m_elxlist.GetSize():  - 1;
    int stp = m_brightleft ?  - 1: 1;
    int eol = m_brightleft ?  - 1: m_elxlist.GetSize();

    // from last
    int n = eol - stp;

    while (n != bol && !m_elxlist[n]->MatchNext(pContext))
    {
        n -= stp;
    }

    if (n != bol)
    {
        n += stp;
    }
    else
    {
        return 0;
    }

    // match rest
    while (n != eol)
    {
        if (m_elxlist[n]->Match(pContext))
        {
            n += stp;
        }
        else
        {
            n -= stp;

            while (n != bol && !m_elxlist[n]->MatchNext(pContext))
            {
                n -= stp;
            }

            if (n != bol)
            {
                n += stp;
            }
            else
            {
                return 0;
            }
        }
    }

    return 1;
}

// mresult.cpp: implementation of the MatchResult class.
//
template <int x> MatchResultT<x>::MatchResultT(CContext *pContext, int nMaxNumber)
{
    if (pContext != 0)
    {
        m_result.Prepare(nMaxNumber *2+3,  - 1);

        // matched
        m_result[0] = 1;
        m_result[1] = nMaxNumber;

        for (int n = 0; n <= nMaxNumber; n++)
        {
            int index = pContext->m_captureindex[n];
            if (index<0)
            {
                continue;
            }

            // check enclosed
            int pos1 = pContext->m_capturestack[index + 1];
            int pos2 = pContext->m_capturestack[index + 2];

            // info
            m_result[n *2+2] = pos1<pos2 ? pos1 : pos2;
            m_result[n *2+3] = pos1<pos2 ? pos2 : pos1;
        }
    }
}

//-------------------------------------------------------------------------

template <int x> inline int MatchResultT<x>::IsMatched()const
{
    return m_result.At(0, 0);
}

//-------------------------------------------------------------------------

template <int x> inline int MatchResultT<x>::MaxGroupNumber()const
{
    return m_result.At(1, 0);
}

//-------------------------------------------------------------------------

template <int x> inline int MatchResultT<x>::GetStart()const
{
    return m_result.At(2,  - 1);
}

//-------------------------------------------------------------------------

template <int x> inline int MatchResultT<x>::GetEnd()const
{
    return m_result.At(3,  - 1);
}

//-------------------------------------------------------------------------

template <int x> inline int MatchResultT<x>::GetGroupStart(int nGroupNumber)const
{
    return m_result.At(2+nGroupNumber * 2,  - 1);
}

//-------------------------------------------------------------------------

template <int x> inline int MatchResultT<x>::GetGroupEnd(int nGroupNumber)const
{
    return m_result.At(2+nGroupNumber * 2+1,  - 1);
}

//-------------------------------------------------------------------------

template <int x> MatchResultT<x>  &MatchResultT<x>::operator = (const MatchResultT<x>  &result)
{
    m_result.Restore(0);
    if (result.m_result.GetSize()> 0)
    {
        m_result.Append(result.m_result.GetBuffer(), result.m_result.GetSize());
    }

    return  *this;
}

// posselx.cpp: implementation of the CPossessiveElx class.
//
template <int x> CPossessiveElxT<x>::CPossessiveElxT(ElxInterface *pelx, int nmin, int nmax): CGreedyElxT<x> (pelx, nmin, nmax){}

template <int x> int CPossessiveElxT<x>::Match(CContext *pContext)const
{
    int nbegin = pContext->m_nCurrentPos;
    int nsize = pContext->m_stack.GetSize();
    int ncsize = pContext->m_capturestack.GetSize();
    int bsucc = 1;

    // match
    if (!CRepeatElxT<x>::MatchFixed(pContext))
    {
        bsucc = 0;
    }
    else
    {
        while (!CGreedyElxT < x > ::MatchVart(pContext))
        {
            if (!CRepeatElxT < x > ::MatchNextFixed(pContext))
            {
                bsucc = 0;
                break;
            }
        }
    }

    // status
    pContext->m_stack.Restore(nsize);

    if (bsucc)
    {
        pContext->m_stack.Push(nbegin);
        pContext->m_stack.Push(ncsize);
    }

    return bsucc;
}

//-------------------------------------------------------------------------

template <int x> int CPossessiveElxT<x>::MatchNext(CContext *pContext)const
{
    int nbegin = 0, ncsize = 0;

    pContext->m_stack.Pop(ncsize);
    pContext->m_stack.Pop(nbegin);

    pContext->m_capturestack.Restore(ncsize);
    pContext->m_nCurrentPos = nbegin;

    return 0;
}

// reluctx.cpp: implementation of the CReluctantElx class.
//
template <int x> CReluctantElxT<x>::CReluctantElxT(ElxInterface *pelx, int nmin, int nmax): CRepeatElxT<x> (pelx, nmin)
{
    m_nvart = nmax - nmin;
}

//-------------------------------------------------------------------------

template <int x> int CReluctantElxT<x>::Match(CContext *pContext)const
{
    if (!CRepeatElxT<x>::MatchFixed(pContext))
    {
        return 0;
    }

    while (!MatchVart(pContext))
    {
        if (!CRepeatElxT<x>::MatchNextFixed(pContext))
        {
            return 0;
        }
    }

    return 1;
}

//-------------------------------------------------------------------------

template <int x> int CReluctantElxT<x>::MatchNext(CContext *pContext)const
{
    if (MatchNextVart(pContext))
    {
        return 1;
    }

    if (!CRepeatElxT<x>::MatchNextFixed(pContext))
    {
        return 0;
    }

    while (!MatchVart(pContext))
    {
        if (!CRepeatElxT<x>::MatchNextFixed(pContext))
        {
            return 0;
        }
    }

    return 1;
}

//-------------------------------------------------------------------------

template <int x> int CReluctantElxT<x>::MatchVart(CContext *pContext)const
{
    pContext->m_stack.Push(0);

    return 1;
}

//-------------------------------------------------------------------------

template <int x> int CReluctantElxT<x>::MatchNextVart(CContext *pContext)const
{
    int n = 0, nbegin = pContext->m_nCurrentPos;

    pContext->m_stack.Pop(n);

    if (n<m_nvart && CRepeatElxT<x>::m_pelx->Match(pContext))
    {
        while (pContext->m_nCurrentPos == nbegin)
        {
            if (!CRepeatElxT<x>::m_pelx->MatchNext(pContext))
            {
                break;
            }
        }

        if (pContext->m_nCurrentPos != nbegin)
        {
            n++;

            pContext->m_stack.Push(nbegin);
            pContext->m_stack.Push(n);

            return 1;
        }
    }

    while (n > 0)
    {
        pContext->m_stack.Pop(nbegin);

        while (CRepeatElxT < x > ::m_pelx->MatchNext(pContext))
        {
            if (pContext->m_nCurrentPos != nbegin)
            {
                pContext->m_stack.Push(nbegin);
                pContext->m_stack.Push(n);

                return 1;
            }
        }

        n--;
    }

    return 0;
}

// repeatx.cpp: implementation of the CRepeatElx class.
//
template <int x> CRepeatElxT<x>::CRepeatElxT(ElxInterface *pelx, int ntimes)
{
    m_pelx = pelx;
    m_nfixed = ntimes;
}

//-------------------------------------------------------------------------

template <int x> int CRepeatElxT<x>::Match(CContext *pContext)const
{
    return MatchFixed(pContext);
}

//-------------------------------------------------------------------------

template <int x> int CRepeatElxT<x>::MatchNext(CContext *pContext)const
{
    return MatchNextFixed(pContext);
}

//-------------------------------------------------------------------------

template <int x> int CRepeatElxT<x>::MatchFixed(CContext *pContext)const
{
    if (m_nfixed == 0)
    {
        return 1;
    }

    int n = 0;

    while (n<m_nfixed)
    {
        if (m_pelx->Match(pContext))
        {
            n++;
        }
        else
        {
            n--;

            while (n >= 0 && !m_pelx->MatchNext(pContext))
            {
                n--;
            }

            if (n >= 0)
            {
                n++;
            }
            else
            {
                return 0;
            }
        }
    }

    return 1;
}

//-------------------------------------------------------------------------

template <int x> int CRepeatElxT<x>::MatchNextFixed(CContext *pContext)const
{
    if (m_nfixed == 0)
    {
        return 0;
    }

    // from last
    int n = m_nfixed - 1;

    while (n >= 0 && !m_pelx->MatchNext(pContext))
    {
        n--;
    }

    if (n >= 0)
    {
        n++;
    }
    else
    {
        return 0;
    }

    // match rest
    while (n<m_nfixed)
    {
        if (m_pelx->Match(pContext))
        {
            n++;
        }
        else
        {
            n--;

            while (n >= 0 && !m_pelx->MatchNext(pContext))
            {
                n--;
            }

            if (n >= 0)
            {
                n++;
            }
            else
            {
                return 0;
            }
        }
    }

    return 1;
}

// Regexp
//typedef CRegexpT <char> CRegexpA;
//typedef CRegexpT <unsigned short> CRegexpW;

#endif
