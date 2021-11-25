//
// (c) 2020 chiv
//
#include "resource.h"
#include "SerialDevsDialog.h"
#include "CodecFactory.h"
#include "ConnFactory.h"
#include "ConnCreateFactory.h"

namespace {
class __class_to_create_conn_serial {
public:
    __class_to_create_conn_serial()
    {
        ConnCreateFactory::Inst()->RegisterInstanceFunc("Serial", "Serial Port", comxd::new_serial(), [=]() {
            SerialDevsDialog d;
            return d.RequestConn();
        });
    }
};
__class_to_create_conn_serial __serial_conn_create;
}

static int kBaudrates[] = { 921600, 460800, 256000, 230400, 153600,
    128000, 115200, 38400, 76800, 57600, 28800, 19200, 14400,
    9600, 7200, 4800, 2400, 1800, 1200 };

SerialDevsDialog::SerialDevsDialog()
{
    CtrlLayout(*this);
    //
    Icon(comxd::new_serial()).Title(t_("Serial Port"));
    // list all ports
    std::vector<serial::PortInfo> ports = serial::list_ports();
    for (size_t k = 0; k < ports.size(); ++k) {
        mDevsList.AddList(ports[k].port.c_str());
    }
    if (!ports.empty()) {
        mDevsList.SetData(mDevsList.Get(0));
    }
    // data bits
    mDataBits.Add(serial::fivebits, "5");
    mDataBits.Add(serial::sixbits, "6");
    mDataBits.Add(serial::sevenbits, "7");
    mDataBits.Add(serial::eightbits, "8");
    mDataBits.SetIndex(3); // 8bits default
    // list baudrate
    for (size_t k = 0; k < ARRAYSIZE(kBaudrates); ++k) {
        mBaudrate.Add(kBaudrates[k], std::to_string(kBaudrates[k]).c_str());
    }
    mBaudrate.SetIndex(6); // default, 115200
    // stop bits
    mStopBits.Add(serial::stopbits_one, "1");
    mStopBits.Add(serial::stopbits_one_point_five, "1.5");
    mStopBits.Add(serial::stopbits_two, "2");
    mStopBits.SetIndex(0); // 1 stop bit default
    // parity
    mParity.Add(serial::parity_none, "None");
    mParity.Add(serial::parity_even, "Even");
    mParity.Add(serial::parity_odd, "Odd");
    mParity.Add(serial::parity_mark, "Mark");
    mParity.Add(serial::parity_space, "Space");
    mParity.SetIndex(0); // default: No parity
    // stream control
    mFlowCtrl.Add(serial::flowcontrol_none, "None");
    mFlowCtrl.Add(serial::flowcontrol_hardware, "Hardware");
    mFlowCtrl.Add(serial::flowcontrol_software, "Software");
    mFlowCtrl.SetIndex(0);
    // types
    auto conn_names = ConnFactory::Inst()->GetSupportedConnNames();
    for (size_t k = 0; k < conn_names.size(); ++k) {
        mTypes.Add(conn_names[k]);
    }
    if (!conn_names.empty()) {
        mTypes.SetIndex(0);
    }
    // CodecFactory MUST have a UTF-8 codec.
    auto codec_names = CodecFactory::Inst()->GetSupportedCodecNames();
    for (size_t k = 0; k < codec_names.size(); ++k) {
        mCodecs.Add(codec_names[k]);
        if (codec_names[k] == "UTF-8" || codec_names[k] == "UTF8") {
            mCodecs.SetIndex((int)k);
        }
    }
    //
    Acceptor(mBtnCancel, IDCANCEL);
    //
    mBtnOk.WhenAction = [=]() { CreateConn(); };
}

bool SerialDevsDialog::Key(Upp::dword key, int count)
{
    dword flags = K_CTRL | K_ALT | K_SHIFT;
    dword d_key = key & ~(flags | K_KEYUP); // key with delta
    flags = key & flags;
    if (key & Upp::K_KEYUP) {
        if (flags == 0 && d_key == Upp::K_ESCAPE) {
            Close();
            return true;
        }
    }
    return TopWindow::Key(key, count);
}

void SerialDevsDialog::ChangeSettings(SerialPort* port)
{
    // load settings of serial
    auto serial = port->GetNativeDevice();
    // name
    mDevsList.SetData(serial->getPort().c_str());
    mDevsList.SetEditable(false);
    // baudrate
    mBaudrate.SetData((int)serial->getBaudrate());
    // stopbits
    mStopBits.SetData(serial->getStopbits());
    // byte size
    mDataBits.SetData(serial->getBytesize());
    // parity
    mParity.SetData(serial->getParity());
    // flow control
    mFlowCtrl.SetData(serial->getFlowcontrol());
    // only change the settings of serial, type could not be modified on running time.
    mTypesLabel.Hide();
    mTypes.Hide();
    mCodecsLabel.Hide();
    mCodecs.Hide();
    //
    int ret = Run(true);
    if (ret == IDOK) {
        try {
            serial->setBaudrate(mBaudrate.GetKey(mBaudrate.GetIndex()).To<int>());
            serial->setParity((serial::parity_t)mParity.GetKey(mParity.GetIndex()).To<int>());
            serial->setBytesize((serial::bytesize_t)mDataBits.GetKey(mDataBits.GetIndex()).To<int>());
            serial->setStopbits((serial::stopbits_t)mStopBits.GetKey(mStopBits.GetIndex()).To<int>());
            serial->setFlowcontrol((serial::flowcontrol_t)mFlowCtrl.GetKey(mFlowCtrl.GetIndex()).To<int>());
        } catch (const std::exception&) {
            Upp::PromptOK(DeQtf(t_("Can't change settings!")));
        }
    }
}
//
void SerialDevsDialog::CreateConn()
{
    try {
        auto serial = std::make_shared<serial::Serial>(
            mDevsList.GetData().ToString().ToStd(),
            mBaudrate.GetKey(mBaudrate.GetIndex()).To<int>(),
            serial::Timeout(),
            (serial::bytesize_t)mDataBits.GetKey(mDataBits.GetIndex()).To<int>(),
            (serial::parity_t)mParity.GetKey(mParity.GetIndex()).To<int>(),
            (serial::stopbits_t)mStopBits.GetKey(mStopBits.GetIndex()).To<int>(),
            (serial::flowcontrol_t)mFlowCtrl.GetKey(mFlowCtrl.GetIndex()).To<int>());
        if (serial) {
            auto port = std::make_shared<SerialPort>(serial);
            port->Start();
            auto conn = ConnFactory::Inst()->CreateInst(~mTypes, port);
            if (!conn) {
                Upp::PromptOK(DeQtf(t_("Dose not support:") + (String)~mTypes));
            } else {
                conn->SetCodec(mCodecs.Get().ToString());
                conn->Start();
                //
                mConn = conn;
                //
                this->AcceptBreak(IDOK);
            }
        }
    } catch (const std::exception&) {
        Upp::PromptOK(t_("Can't open, because:") + mDevsList.GetData().ToString() + "&" + t_("-|1. Not supported settings") + "&" + t_("-|2. Device was opened already"));
    }
}
// This is a factory, it will create kingds conn according to the current settings.
SerialConn* SerialDevsDialog::RequestConn()
{
    int ret = Run(true);
    if (ret == IDOK) {
        return mConn;
    }
    return nullptr;
}
