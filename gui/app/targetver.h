// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.


#ifndef GUI_TARGETVER_H_
#define GUI_TARGETVER_H_

// ���º궨��Ҫ������ƽ̨��Ҫ������ƽ̨
// �Ǿ�������Ӧ�ó������蹦�ܵ� Windows��Internet Explorer �Ȳ�Ʒ��
// ����汾��ͨ����ָ���汾�����Ͱ汾��ƽ̨���������п��õĹ��ܣ������
// ����������

// �������Ҫ��Ե�������ָ���汾��ƽ̨�����޸����ж��塣
// �йز�ͬƽ̨��Ӧֵ��������Ϣ����ο� MSDN��
#ifndef WINVER              // ָ��Ҫ������ƽ̨�� Windows Vista��
#define WINVER 0x0600       // ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif  // WINVER

#ifndef _WIN32_WINNT      // ָ��Ҫ������ƽ̨�� Windows Vista��
#define _WIN32_WINNT 0x0600   // ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif  // _WIN32_WINNT

#ifndef _WIN32_WINDOWS      // ָ��Ҫ������ƽ̨�� Windows 98��
#define _WIN32_WINDOWS 0x0410 // ����ֵ����Ϊ�ʵ���ֵ���������� Windows Me ����߰汾��
#endif  // _WIN32_WINDOWS

#ifndef _WIN32_IE             // ָ��Ҫ������ƽ̨�� Internet Explorer 7.0��
#define _WIN32_IE 0x0700    // ����ֵ����Ϊ��Ӧ��ֵ���������� IE �������汾��
#endif  // _WIN32_IE
#endif  // GUI_TARGETVER_H_
