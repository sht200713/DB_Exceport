#include "dbexport.h"
#include <iostream>
#include <fstream>
#include <QFileDialog>
#include <ActiveQt/QAxObject>
#include  <QDesktopServices>
#include "OptDlg.h"
dbexport::dbexport()
{
	m_model = new QSqlQueryModel();
	db = QSqlDatabase::addDatabase("QSQLITE");
}
 dbexport::~dbexport()
 {
	 if (db.isOpen()) db.close();
 }
 void dbexport::init()
 {
	 connect(pb_OpenDB, SIGNAL(clicked()),this, SLOT(opendb()));
	 connect(pb_Export, SIGNAL(clicked()),this, SLOT(exportex()));
 }
bool dbexport::createConnection(QString dbfile)
{
  db.setDatabaseName(dbfile);
  if(!db.open()){
    qDebug() << "Database Open Error!"; return false;
  }
  QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
  QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
  return true;
}
void dbexport::opendb()
{
	std::cout<<"opendb"<<std::endl;
	QString fileName = QFileDialog::getOpenFileName(0,
		tr("Open DB"), "./", tr("db Files (*.db3)"));
	//QString fileName="data-057.db3";
	qDebug()<<fileName;
	if (fileName.isEmpty())return;
	if (db.isOpen())db.close();
	if(!createConnection(fileName)) return;
	le_db->setText(fileName);
	OptDlg dlg(Optwidget);
	int res = dlg.exec();
	QString name,postision;
	if(res == QDialog::Accepted)
	{
		b_sgns=dlg.buttonGroup->checkedId();
	}
	if (b_sgns==0)
	{
		m_model->setQuery(QString("select patient.NAME,screws.NAME as SNAME,screws.LEN,screws.D,screws.PLOTX,screws.PLOTY from screws left join surgerys on surgerys.ID=screws.SURGERY left join patient on surgerys.PID=patient.ID"));
		m_model->setHeaderData(0, Qt::Horizontal, QObject::tr("����"));
		m_model->setHeaderData(1, Qt::Horizontal, QObject::tr("����λ��"));
		m_model->setHeaderData(2, Qt::Horizontal, QObject::tr("���ӳ���"));
		m_model->setHeaderData(3, Qt::Horizontal, QObject::tr("����ֱ��"));
		tableView->setModel(m_model);
		tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
		tableView->horizontalHeader()->setStretchLastSection(true);
		tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
		tableView->setColumnHidden(4,true);
		tableView->setColumnHidden(5,true);
	}else if(b_sgns==1)
	{
		/*m_model->setQuery(QString("select NAME,POS,TIME,PLOTY from gsns"));*/
		QString querys="select patient.NAME,screws.NAME as SNAME,screws.LEN,screws.D,screws.PLOTX,screws.PLOTY,screws.PLOTT,screws.PLOTS,screws.PLOTD from screws left join surgerys on surgerys.ID=screws.SURGERY left join patient on surgerys.PID=patient.ID";
		m_model->setQuery(querys);
		m_model->setHeaderData(0, Qt::Horizontal, QObject::tr("����"));
		m_model->setHeaderData(1, Qt::Horizontal, QObject::tr("����λ��"));
		tableView->setModel(m_model);
		tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
		tableView->horizontalHeader()->setStretchLastSection(true);
		tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
		for (int i=0;i<5;i++) tableView->setColumnHidden(4+i,true);
	}

}
void dbexport::exporttxt()
{
	QString sexcel=le_excel->text();
	QSqlRecord record;
	QByteArray plotx,ploty;
	QString patienName,dingName;
	std::ofstream ofile;
	ofile.open((sexcel+"11.txt").toStdString().c_str(), std::ios::out);
	if(!ofile) return ;
	for(int i=0; i<this->m_model->rowCount(); i++){
		xs.clear();
		yl.clear();
		record = m_model->record(i);
		//qDebug()<<record.fieldName(0)<<record.fieldName(1)<<record.fieldName(2);
		patienName=record.value("NAME").toString();
		dingName=record.value("SNAME").toString();
		plotx=record.value("PLOTX").toByteArray();
		ploty=record.value("PLOTY").toByteArray();
		QDataStream streamx(plotx);
		streamx >> xs;
		QDataStream streamy(ploty);
		streamy >> yl;
		int vsize=xs.size();
		std::cout<<"vsize:"<<vsize<<std::endl;
		if(vsize<1)continue;
		ofile<<patienName.toStdString().c_str()<<" "<<dingName.toStdString().c_str()<<" "<<"�г�";
		for (int j=0;j<vsize;j++) 
		{
			if (j!=0&&j%500==0) 
			{
				ofile<<"\n";
				ofile<<patienName.toStdString().c_str()<<" "<<dingName.toStdString().c_str()<<" "<<"�г�";
			}
			ofile<<" "<<xs[j];
		}
		ofile<<"\n";
		ofile<<patienName.toStdString().c_str()<<" "<<dingName.toStdString().c_str()<<" "<<"ѹ��";
		for (int j=0;j<vsize;j++) 
		{
			if (j!=0&&j%500==0)
			{
				ofile<<"\n";
				ofile<<patienName.toStdString().c_str()<<" "<<dingName.toStdString().c_str()<<" "<<"ѹ��";
			}
			ofile<<" "<<yl[j];
		}
		ofile<<"\n";
	}
	ofile.close();
}
QString to26AlphabetString(int data)
{
	QChar ch = data + 0x40;//A��Ӧ0x41
	return QString(ch);
}
void convertToColName(int data, QString &res)//��bug�����õڶ��ַ�ʽ����excel�к�excelColIndexToStr
{
	//qDebug()<<"covertdata:"<<data;
	Q_ASSERT(data>0 && data<65535);
	
	//Q_ASSERT(data>0);

	int tempData = data / 26;
	//std::cout<<"tempData:"<<tempData<<std::endl;
	//qDebug()<<"tempData:"<<tempData;
	if(tempData > 1)
	{
		int mode = data % 26;
		//std::cout<<"mode:"<<mode<<std::endl;
		if(mode==0) 
		{
			res=(to26AlphabetString(26)+res);
			convertToColName(tempData-1,res);
		}
		else
		{
			convertToColName(mode,res);
			convertToColName(tempData,res);
		}
	}
	else
	{
		res=(to26AlphabetString(data)+res);
	}
}

void  excelColIndexToStr(int columnIndex,QString &columnStr) {
	     if (columnIndex <= 0) columnStr="";
	     columnIndex--;
	     do {
	             if (columnStr.length() > 0) 
	                 columnIndex--;
	             columnStr = ((char) (columnIndex % 26 + (int) 'A')) + columnStr;
	             columnIndex = (int) ((columnIndex - columnIndex % 26) / 26);
	         } while (columnIndex > 0);
}

void Excel_SetCell(QAxObject *worksheet,int column,int row,QColor color,QString text)
{
	QAxObject *cell = worksheet->querySubObject("Cells(int,int)", row, column);
	cell->setProperty("Value", text);
	QAxObject *font = cell->querySubObject("Font");
	font->setProperty("Color", color);
}
QString saveas()
{
	QString file;
	QString filter;


	//����汾����QT5,����Ҫ��:
	//  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
	//��Ϊ:QDesktopServices::storageLocation(QDesktopServices::DesktopLocation),
	file = QFileDialog::getSaveFileName (
		NULL,                               //�����
		"���Ϊ",                              //����
		//QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),                 //����·��, .��ʾ��ǰ·��,./��ʾ��Ŀ¼
		QDesktopServices::storageLocation(QDesktopServices::DesktopLocation),
		"Excel(*.xlsx)",     //������
		&filter  );

	return file;
}
//��QList<QList<QVariant> > תΪQVariant,���ڿ���д���
void castListListVariant2Variant(const QList<QList<QVariant> > &cells, QVariant &res)
{
	QVariantList vars;
	const int rows = cells.size();
	for(int i=0;i<rows;++i)
	{
		vars.append(QVariant(cells[i]));
	}
	res = QVariant(vars);
}

void dbexport::exportex()
{
	QString templatePath = "./db.xlsx";
	QFileInfo info(templatePath);
	if(!info.exists())
	{
		qDebug()<<"db.xlsx is NULL";
		return ;
	}
	templatePath = info.absoluteFilePath();                   //��ȡģ��ľ���·��
	templatePath = QDir::toNativeSeparators(templatePath);   //ת��һ��·��,��windows�ܹ�ʶ��

	QString ExcelFile = QDir::toNativeSeparators(saveas());  //���ļ�����Ի���,�ҵ�Ҫ�����λ��
	if(ExcelFile=="") return;
	le_excel->setText(ExcelFile);
	QFile::copy(templatePath, ExcelFile);                   //��ģ���ļ����Ƶ�Ҫ�����λ��ȥ

	info.setFile(ExcelFile);
	info.setFile(info.dir().path()+"/~$"+info.fileName());

	if(info.exists())          //�ж�һ��,��û��"~$XXX.xlsx"�ļ�����,�ǲ���Ϊֻ��
	{
		qDebug()<<"��������Ϊֻ��,�����ļ��Ƿ��Ѵ�!";
		return   ;
	}

	QAxObject *excel = new QAxObject();//����excel��������
	excel->setControl("Excel.Application");//����Excel�ؼ�
	excel->setProperty("Visible", true);//��ʾ���忴Ч��
	excel->setProperty("DisplayAlerts", false);//��ʾ���濴Ч��
	QAxObject *workbooks = excel->querySubObject("WorkBooks");
	QString sexcel=le_excel->text()+"db.xlsx";
	QSqlRecord record;
	QByteArray plotx,ploty,plott,plots,plotd;
	QString patienName,dingName;
	QAxObject* workbook = workbooks->querySubObject("Open(const QString&)",QDir::toNativeSeparators(ExcelFile));
	excel->setProperty("Caption", "db_Excel");      //����ΪQt Excel
	QAxObject *work_book = excel->querySubObject("ActiveWorkBook");
	QAxObject *worksheet = work_book->querySubObject("Sheets(int)",1); 
	int nn=2;
	for(int i=0; i<this->m_model->rowCount(); i++){
		xs.clear();
		yl.clear();
		record = m_model->record(i);
		//qDebug()<<record.fieldName(0)<<record.fieldName(1)<<record.fieldName(2);
		if (b_sgns==0)
		{
			patienName=record.value("NAME").toString();
			dingName=record.value("SNAME").toString();
			plotx=record.value("PLOTX").toByteArray();
			ploty=record.value("PLOTY").toByteArray();
			QDataStream streamx(plotx);
			streamx >> xs;
			QDataStream streamy(ploty);
			streamy >> yl;
			int vsize=xs.size();
			std::cout<<"vsize:"<<vsize<<std::endl;
			Excel_SetCell(worksheet,1,i+nn,QColor(0,0,255),patienName);
			Excel_SetCell(worksheet,2,i+nn,QColor(0,0,255),dingName);
			Excel_SetCell(worksheet,3,i+nn,QColor(0,0,255),"�г�");
			nn++;
			Excel_SetCell(worksheet,3,i+nn,QColor(0,0,255),"ѹ��");
			if(vsize<1)continue;
		}else if (b_sgns==1)
		{

			yt.clear();
			ys.clear();
			yd.clear();
	

			patienName=record.value("NAME").toString();
			dingName=record.value("POS").toString();
			plotx=record.value("TIME").toByteArray();
			ploty=record.value("PLOTY").toByteArray();
			plott=record.value("PLOTT").toByteArray();
			plots=record.value("PLOTS").toByteArray();
			plotd=record.value("PLOTD").toByteArray();

			QDataStream streamx(plotx);
			streamx >> xs[0];


			QDataStream streamy(ploty);
			streamy >> yl[0];
			QDataStream streamt(plott);
			streamt >> yt[0];
			QDataStream streams(plots);
			streams >> ys;
			QDataStream streamd(plotd);
			streamd >> yd;
			int vsize=xs.size();
			std::cout<<"vsize:"<<vsize<<std::endl;
			Excel_SetCell(worksheet,1,i*5+2,QColor(0,0,255),patienName);
			Excel_SetCell(worksheet,2,i*5+2,QColor(0,0,255),dingName);
			Excel_SetCell(worksheet,3,i*5+2,QColor(0,0,255),"�г�");
			Excel_SetCell(worksheet,3,i*5+3,QColor(0,0,255),"ѹ��");
			Excel_SetCell(worksheet,3,i*5+4,QColor(0,0,255),"Ť��");
			Excel_SetCell(worksheet,3,i*5+5,QColor(0,0,255),"�ٶ�");
			Excel_SetCell(worksheet,3,i*5+6,QColor(0,0,255),"ת��");
			if(vsize<1)continue;
		}
		//������Ԫ��洢
#if 0	
		for (int j=0;j<vsize;j++) 
		{
			char buffer[20];
			sprintf_s(buffer,10,"%f",xs[j]);
			QString str=buffer;
			Excel_SetCell(worksheet,4+j,i+nn,QColor(0,0,0),str);
		}
		nn++;
		Excel_SetCell(worksheet,3,i+nn,QColor(255,0,0),"ѹ��");
		for (int j=0;j<vsize;j++) 
		{
			char buffer[20];
			sprintf_s(buffer,10,"%f",y1[j]);
			QString str=buffer;
			Excel_SetCell(worksheet,4+j,i+nn,QColor(0,0,0),str);
		}
#else 
		//�����洢
		//QAxObject *user_range = worksheet->querySubObject("Range(const QString&)","A4:I106");
		QString rangStr;
		QList<QList<QVariant>> cells;
		if (b_sgns==1)
		{
			QList<QVariant> clodata1,clodata2,clodatat,clodatas,clodatad;
			int xssize=xs.size();
			for (int jj=0;jj<xssize;jj++)
			{
				clodata1.push_back(xs[jj]);
				clodata2.push_back(yl[jj]);
				clodatat.push_back(yt[jj]);
				clodatas.push_back(ys[jj]);
				clodatad.push_back(yd[jj]);
			}	
			cells.append(clodata1);
			cells.append(clodata2);
			cells.append(clodatat);
			cells.append(clodatas);
			cells.append(clodatad);
			int row = cells.size();
			int col = cells.at(0).size()+3;
			//qDebug()<<col;
			
			//convertToColName(col,rangStr);//�к�
			excelColIndexToStr(col,rangStr);
			rangStr += QString::number(2+i*5+5);//�к�
			rangStr = "D"+QString::number(2+i*5)+":"+ rangStr;

		}else
		{	
			QList<QVariant> clodata1,clodata2;
			int xssize=xs.size();
			for (int jj=0;jj<xssize;jj++)
			{
				clodata1.push_back(xs[jj]);
				clodata2.push_back(yl[jj]);
			}	
			cells.append(clodata1);
			cells.append(clodata2);

			int row = cells.size();
			int col = cells.at(0).size()+3;
			//qDebug()<<col;
			//convertToColName(col,rangStr);//�к�
			excelColIndexToStr(col,rangStr);
			rangStr += QString::number(2+i*2+1);//�к�
			rangStr = "D"+QString::number(2+i*2)+":"+ rangStr;

		}
		qDebug()<<rangStr;
		QAxObject *range = worksheet->querySubObject("Range(const QString&)",rangStr);
		if(NULL == range || range->isNull()) return;
		QVariant var;
		castListListVariant2Variant(cells,var);
		range->setProperty("Value", var);
		delete range;
#endif
	}
	workbook->dynamicCall("Save()" );
	workbook->dynamicCall("Close()");  //�ر��ļ�
	excel->dynamicCall("Quit()");//�ر�excel
	delete worksheet;
	delete work_book;
	delete workbook;
	delete workbooks;
	delete excel;
	qDebug()<<"Convert Success!";
}

