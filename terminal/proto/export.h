/*!
// (c) 2022 chiv
//
*/
#pragma once

class TransmitProgressDialog;
class ProgressDialog {
public:
	ProgressDialog();
	virtual ~ProgressDialog();
	void SetTotal(Upp::int64);
	void Update(Upp::int64 bytes, double rate);
	void Close();
	
	int Run(bool modal = true);
private:
	TransmitProgressDialog* mImpl;
};

