/*
    Simple test for MIPSim
    
    * run mipsim and gdb sim in parallel
    * load the same target program in both simulators
    * run the program step by step
    * check for disparity in the effect of each instruction
    * report issues
*/

#include <QDir>
#include <QDebug>
#include <QString>
#include <QProcess>

enum {
    GDB,
    MIPSim,
    
    TARGET_COUNT
};

enum Regnames {
    
    // cpu GPR
    ZERO,
    AT,
    V0,
    V1,
    A0,
    A1,
    A2,
    A3,
    T0,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    S0,
    S1,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    T8,
    T9,
    K0,
    K1,
    GP,
    SP,
    FP,
    RA,
    
    // cpu SPR
    PC,
    IR,
    
    HI,
    LO,
    
    
    NONE = -1
};

struct RegisterFile {
    quint32 pc;
    quint32 hi, lo;
    quint32 gpr[32];
};

static RegisterFile regs[TARGET_COUNT];

void print_register_file_diff(int t1, int t2)
{
    if ( regs[t1].pc != regs[t2].pc )
        qDebug("pc : 0x%08x != 0x%08x", regs[t1].pc, regs[t2].pc);
    
    for ( int i = 0; i < 32; ++i )
        if ( regs[t1].gpr[i] != regs[t2].gpr[i] )
            qDebug("r%i : 0x%08x != 0x%08x", i, regs[t1].gpr[i], regs[t2].gpr[i]);
    
    if ( regs[t1].hi != regs[t2].hi )
        qDebug("hi : 0x%08x != 0x%08x", regs[t1].hi, regs[t2].hi);
    
    if ( regs[t1].lo != regs[t2].lo )
        qDebug("lo : 0x%08x != 0x%08x", regs[t1].lo, regs[t2].lo);
    
}

static QProcess* sim[TARGET_COUNT];
static const QByteArray prompts[TARGET_COUNT] = {
    "(gdb) ", "mipsim> "
};

bool at_prompt[TARGET_COUNT];

QByteArray command(int target, const QByteArray& command)
{
    QByteArray tmp;
    QProcess *p = sim[target];
    const QByteArray& prompt = prompts[target];
    
    while ( !at_prompt[target] )
    {
        p->waitForReadyRead(100);
        tmp += p->readAll();
        
        at_prompt[target] = tmp.endsWith(prompt);
    }
    
    //qDebug("%d> %s", target, qPrintable(command));
    
    p->write(command);
    p->waitForBytesWritten(100);
    
    QByteArray ans;
    at_prompt[target] = false;
    
    while ( !at_prompt[target] && (p->state() == QProcess::Running) )
    {
        p->waitForReadyRead(100);
        ans += p->readAll();
        
        if ( ans.endsWith(prompt) )
        {
            ans.chop(prompt.length());
            at_prompt[target] = true;
        }
    }
    
    return ans;
}

static const QByteArray reg_dump_cmd[TARGET_COUNT] = {
    "info registers\n",
    "d\n"
};

static const QByteArray reg_dump_skip[TARGET_COUNT] = {
    "The program has no registers now.\n",
    "dummy!"
};

static const QList<int> reg_dump_mapping[TARGET_COUNT] = {
    QList<int>()
    <<  9 << 10 << 11 << 12 << 13 << 14 << 15 << 16
    << 26 << 27 << 28 << 29 << 30 << 31 << 32 << 33
    << 43 << 44 << 45 << 46 << 47 << 48 << 49 << 50
    << 60 << 61 << 62 << 63 << 64 << 65 << 66 << 67
    << 79 << -1
    << 76 << 75
    ,
    QList<int>()
    << 11 << 14 << 17 << 20 << 23 << 26 << 29 << 32
    << 35 << 38 << 41 << 44 << 47 << 50 << 53 << 56
    << 59 << 62 << 65 << 68 << 71 << 74 << 77 << 80
    << 83 << 86 << 89 << 92 << 95 << 98 << 101<< 104
    <<  2 << -1
    <<  5 <<  8
};

typedef QPair<int, int> QRange;
typedef QList<QRange> QRangeList;

void split_ws(QByteArray s, QRangeList& l)
{
    int idx, last;
    
    idx = last = 0;
    const int len = s.length();
    const char *d = s.constData();
    
    while ( idx < len )
    {
        if ( isspace(d[idx]) )
        {
            if ( last != idx )
                l << qMakePair(last, idx - last);
            
            last = ++idx;
        } else {
            ++idx;
        }
    }
    
    if ( last != idx )
        l << qMakePair(last, idx - last);
    
}

quint32 hex_value(const char *d, int length, bool *ok)
{
    quint32 n = 0;
    
    if ( ok ) *ok = false;
    
    if ( length > 2 && d[0] == '0' && d[1] == 'x' )
    {
        length -= 2;
        d += 2;
    }
    
    do {
        char c = *d;
        
        n <<= 4;
        
        if ( c >= '0' && c <= '9' )
            n += c - '0';
        else if ( c >= 'a' && c <= 'f' )
            n += c - 'a' + 10;
        else if ( c >= 'A' && c <= 'F' )
            n += c - 'A' + 10;
        else
            return 0;
        
        ++d;
    } while ( --length );
    
    if ( ok ) *ok = true;
    
    return n;
}

QRangeList ranges[TARGET_COUNT];

void update_register_file(int target)
{
    QByteArray ans = command(target, reg_dump_cmd[target]);
    
    if ( ans == reg_dump_skip[target] )
        return;
    
    int n = 0;
    split_ws(ans, ranges[target]);
    
    for ( int i = 0; i < reg_dump_mapping[target].count(); ++i )
    {
        int idx = reg_dump_mapping[target].at(i);
        
        if ( idx >= 0 )
        {
            if ( idx < ranges[target].count() )
            {
                bool ok = false;
                QRange r = ranges[target].at(idx);
                quint32 val = hex_value(ans.constData() + r.first, r.second, &ok);
                
                if ( !ok ) {
                    qWarning("(%i) borked reg dump : %i = %s", target, i, ans.mid(r.first, r.second).constData());
                } else {
                    ++n;
                    
                    if ( i >= 0 && i < 32 )
                        regs[target].gpr[i] = val;
                    else if ( i == PC )
                        regs[target].pc = val;
                    else if ( i == HI )
                        regs[target].hi = val;
                    else if ( i == LO )
                        regs[target].lo = val;
                    else
                        --n;
                }
            } else {
                qDebug("doh!");
            }
        }
    }
    //qDebug("dumped %d registers from target %d", n, target);
}

int main(int argc, char **argv)
{
    QString target, ans;
    QProcess mipsim, gdb;
    
    sim[GDB]    = &gdb;
    sim[MIPSim] = &mipsim;
    
    at_prompt[GDB] = at_prompt[MIPSim] = false;
    
    target = QString::fromLocal8Bit(argv[1]);
    
    mipsim.setProcessChannelMode(QProcess::MergedChannels);
    mipsim.start("./mipsim", QStringList() << target);
    
    gdb.setProcessChannelMode(QProcess::MergedChannels);
    gdb.start("mips-elf-gdb", QStringList() << target << "--silent");
    
    if ( !gdb.waitForStarted() )
    {
        qDebug("Unable to start gdb");
        return 1;
    }
    
    
    // get gdb simulator ready for single stepping...
    command(GDB, "target sim\n");
    command(GDB, "load\n");
    command(GDB, "b _start\n");
    command(GDB, "r\n");
    
    qDebug("Initial register dump");
    
    memset(regs, 0, sizeof(RegisterFile) * TARGET_COUNT);
    
    update_register_file(GDB);
    update_register_file(MIPSim);
    
    while ( regs[GDB].pc < regs[MIPSim].pc )
    {
        command(GDB, "si\n");
        update_register_file(GDB);
    }
    
    while ( regs[MIPSim].pc < regs[GDB].pc )
    {
        command(MIPSim, "s\n");
        update_register_file(MIPSim);
    }
    
    qDebug("Starting @ 0x%08x, 0x%08x", regs[GDB].pc, regs[MIPSim].pc);
    
    int n = 0, diff_seq = 0;
    const int max_diff_seq = QString::fromLocal8Bit(argv[2]).toUInt();
    
    forever
    {
        if ( command(GDB, "info program\n") == "The program being debugged is not being run.\n" )
        {
            qDebug("sim target stopped");
            break;
        }
        
        if ( memcmp(&regs[GDB], &regs[MIPSim], sizeof(RegisterFile)) )
        {
            if ( !diff_seq )
            {
                // account for GDB maybe lagging one instruction behind
                quint32 pc = regs[GDB].pc;
                
                if ( regs[MIPSim].pc - pc == 4 )
                {
                    command(GDB, "si\n");
                    update_register_file(GDB);
                    
                    if ( memcmp(&regs[GDB], &regs[MIPSim], sizeof(RegisterFile)) )
                    {
                        qDebug("Simulations differ after %i instruction : ", n);
                        qDebug(" @ 0x%08x, 0x%08x", pc, regs[MIPSim].pc);
                        print_register_file_diff(GDB, MIPSim);
                        
                        ++n;
                        ++diff_seq;
                        
                        command(MIPSim, "s\n");
                        update_register_file(MIPSim);
                        
                        continue;
                    } else {
                        //qDebug("Corrected delay slot GDB lag @ 0x%08x", pc);
                        continue;
                    }
                }
                
                qDebug("Simulations differ after %i instruction : ", n);
                qDebug(" @ 0x%08x, 0x%08x", pc, regs[MIPSim].pc);
                print_register_file_diff(GDB, MIPSim);
            }
            
            if ( ++diff_seq >= max_diff_seq )
            {
                qDebug("simulations not merged after %i steps : aborting...", diff_seq);
                print_register_file_diff(GDB, MIPSim);
                break;
            }
        } else if ( diff_seq ) {
            qDebug("simulations merge back after %i steps", diff_seq);
            diff_seq = 0;
        }
        
        command(MIPSim, "s\n");
        update_register_file(MIPSim);
        
        command(GDB, "si\n");
        update_register_file(GDB);
        
        ++n;
    }
    
    command(GDB, "q\ny\n");
    
    command(MIPSim, "q\n");
}
