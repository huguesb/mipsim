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
    QByteArray ans;
    QProcess *p = sim[target];
    const QByteArray& prompt = prompts[target];
    
    while ( !at_prompt[target] )
    {
        p->waitForReadyRead();
        ans += p->readAll();
        
        at_prompt[target] = ans.endsWith(prompt);
    }
    
    //qDebug("%d> %s", target, command.constData());
    
    p->write(command);
    p->waitForBytesWritten();
    
    ans.clear();
    at_prompt[target] = false;
    
    bool chop_readline = true;
    
    while ( !at_prompt[target] && (p->state() == QProcess::Running) )
    {
        p->waitForReadyRead();
        ans += p->readAll();
        
        if ( chop_readline && ans.startsWith(command) )
        {
            ans.remove(0, command.length());
            chop_readline = false;
        }
        
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

typedef QVector<int> QRangeList;

void split_ws(QByteArray s, QRangeList& l)
{
    int idx, last;
    
    idx = last = 0;
    const int len = s.length();
    const char *d = s.constData();
    
    while ( idx < len )
    {
        if ( d[idx] > ' ' )
        {
            ++idx;
        } else {
            if ( last != idx )
                l << last << (idx - last);
            
            last = ++idx;
        }
    }
    
    if ( last != idx )
        l << last << (idx - last);
    
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
        unsigned char c = *((unsigned char*)d);
        
        if ( c >= '0' && c <= '9' )
            c -= '0';
        else if ( c >= 'A' && c <= 'F' )
            c += 10 - 'A';
        else if  ( c >= 'a' && c <= 'f' )
            c += 10 - 'a';
        else
            return 0;
        
        n <<= 4;
        n += c;
        
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
            if ( (2*idx+1) < ranges[target].count() )
            {
                bool ok = false;
                int r_f = ranges[target].at(2*idx);
                int r_s = ranges[target].at(2*idx+1);
                quint32 val = hex_value(ans.constData() + r_f, r_s, &ok);
                
                if ( !ok ) {
                    qWarning("(%i) borked reg dump : %i = %s", target, i, ans.mid(r_f, r_s).constData());
                    qDebug() << ans << endl << ranges[target];
                    exit(1);
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
    if ( argc < 2 )
    {
        qDebug("usage : cmpsim <target> [max_branch_length]");
        return 1;
    }
    
    QString target, ans;
    QProcess mipsim, gdb;
    
    sim[GDB]    = &gdb;
    sim[MIPSim] = &mipsim;
    
    at_prompt[GDB] = at_prompt[MIPSim] = false;
    
    target = QString::fromLocal8Bit(argv[1]);
    
    mipsim.setProcessChannelMode(QProcess::MergedChannels);
    mipsim.start("./simips", QStringList() << target << "--zero-sp");
    
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
    
    //qDebug("Ready @ 0x%08x, 0x%08x", regs[GDB].pc, regs[MIPSim].pc);
    
    //print_register_file_diff(GDB, MIPSim);
    
    if ( regs[GDB].pc == 0xbfc00000 )
    {
        qDebug("Unable to load target");
    } else {
        while ( regs[GDB].pc < regs[MIPSim].pc )
        {
            command(GDB, "si\n");
            update_register_file(GDB);
        }
        
        while ( regs[MIPSim].pc < regs[GDB].pc )
        {
            command(MIPSim, "si\n");
            update_register_file(MIPSim);
        }
        
        qDebug("Starting @ 0x%08x, 0x%08x", regs[GDB].pc, regs[MIPSim].pc);
        
        int n = 0, diff_seq = 0;
        const int max_diff_seq = QString::fromLocal8Bit(argv[2]).toUInt();
        
        forever
        {
//             if ( !(n % 100) )
//                qDebug("%d instructions executed", n);
            
            bool gdb_stop = false, mipsim_stop = false;
            
            if ( command(GDB, "info program\n") == "The program being debugged is not being run.\n" )
            {
                gdb_stop = true;
                qDebug("[GDB] sim target stopped");
            }
            
            QByteArray stat = command(MIPSim, "status\n").trimmed();
            
            if ( stat.count() )
            {
                mipsim_stop = true;
                qDebug("[MIPSim] sim target stopped : %s", stat.constData());
            }
            
            if ( mipsim_stop || gdb_stop )
            {
                if ( !(mipsim_stop && gdb_stop) )
                    qDebug("Only one of the simulator stopped...");
                
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
                            
                            command(MIPSim, "si\n");
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
            
            command(MIPSim, "si\n");
            update_register_file(MIPSim);
            
            command(GDB, "si\n");
            update_register_file(GDB);
            
            ++n;
        }
        
        qDebug("Stopped after %d steps", n);
    }
    
    command(GDB, "q\ny\n");
    
    command(MIPSim, "q\n");
}
