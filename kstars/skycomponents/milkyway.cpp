/***************************************************************************
                          milkyway.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : 12 Nov. 2005
    copyright            : (C) 2005 by Jason Harris
    email                : kstars@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QList>
#include <QPointF>
#include <QPolygonF>
#include <QPainter>

#include <klocale.h>

#include "kstars.h"
#include "kstarsdata.h"
#include "skymap.h"
#include "skypoint.h"
#include "dms.h"
#include "Options.h"
#include "ksfilereader.h"

#include "skymesh.h"

#include "milkyway.h"


MilkyWay::MilkyWay( SkyComponent *parent ) : 
    SkipListIndex( parent, "Milky Way" )
{}

void MilkyWay::init( KStarsData *data )
{
    intro();

    char* fname = "milkyway.dat";
	QString line;
    double ra, dec, lastRa, lastDec;
	SkipList *skipList = 0;
    bool ok;
    int iSkip = 0;

    lastRa = lastDec = -1000.0;

	KSFileReader fileReader;
    if ( ! fileReader.open( fname ) ) return;

	while ( fileReader.hasMoreLines() ) {
		line = fileReader.readLine();

        QChar firstChar = line.at( 0 );
        if ( firstChar == '#' ) continue; 

        ra = line.mid( 2, 8 ).toDouble(&ok);        
		if ( ok ) dec = line.mid( 11, 8 ).toDouble(&ok);
        if ( !ok ) {
            fprintf(stderr, "%s: conversion error on line: %d\n",
                    fname, fileReader.lineNumber());
            continue;
        }

		if ( firstChar == 'M' )  {
			if (  skipList )  appendBoth( skipList );
			skipList = 0;
            iSkip = 0;
            lastRa = lastDec = -1000.0;
		}

        if ( ! skipList ) skipList = new SkipList();

        if ( ra == lastRa && dec == lastDec ) {
            fprintf(stderr, "%s: tossing dupe on line %4d: (%f, %f)\n",
                    fname, fileReader.lineNumber(), ra, dec);
            continue;
        }

		skipList->points()->append( new SkyPoint(ra, dec) );
        lastRa = ra;
        lastDec = dec;
		if ( firstChar == 'S' ) skipList->setSkip( iSkip );
        iSkip++;
	}
    if ( skipList ) appendBoth( skipList );

    summary();
        //printf("Done.\n");
}

bool MilkyWay::selected() 
{
    return Options::showMilkyWay();
}

// We interate over the values in the poly/lineIndex so we don't
// need to keep a separate list of lineLists just to do the updates
// -- jbb
void MilkyWay::update( KStarsData *data, KSNumbers *num )
{
    return;  // FIXME: -jbb just remove the whole routine.
    if ( ! selected() ) return;

    skyMesh()->incDrawID();
    DrawID drawID = skyMesh()->drawID();

    foreach (LineListList* listList, polyIndex()->values() ) {
        for ( int i = 0; i < listList->size(); i++) {
            LineList* lineList = listList->at( i );

            if ( lineList->drawID == drawID ) continue;
            lineList->drawID = drawID;

            SkyList* points = lineList->points();
            for (int j = 0; j < points->size(); j++ ) {
                SkyPoint* p = points->at( j );
                if ( num ) p->updateCoords( num );
                p->EquatorialToHorizontal( data->lst(), data->geo()->lat() );
            }
        }
    }
}

void MilkyWay::draw(KStars *kstars, QPainter& psky, double scale)
{
	if ( !selected() ) return;

    QColor color =  QColor( kstars->data()->colorScheme()->colorNamed( "MWColor" ) );

	psky.setPen( QPen( color, 3, Qt::SolidLine ) );
    psky.setBrush( QBrush( color ) );

	// Uncomment these two lines to get more visible images for debugging.  -jbb
	//
	//psky.setPen( QPen( QColor( "red" ), 1, Qt::SolidLine ) );
	//psky.setBrush( QBrush( QColor("green"  ) ) );

    if ( Options::fillMilkyWay() ) {
        if ( Options::useAntialias() ) 
        	drawFilledFloat( kstars, psky, scale );
        else
        	drawFilledInt( kstars, psky, scale );
    }
    else {
        if ( Options::useAntialias() ) 
        	drawLinesFloat( kstars, psky, scale );        
        else
        	drawLinesInt( kstars, psky, scale );
    }
}

