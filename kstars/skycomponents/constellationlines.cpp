/***************************************************************************
                 constellationlines.h  -  K Desktop Planetarium
                             -------------------
    begin                : 25 Oct. 2005
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

#include "constellationlines.h"
#include "linelist.h"

#include <QPen>
#include <QPainter>

#include <kdebug.h>
#include <klocale.h>

#include "kstars.h"
#include "Options.h"
#include "kstarsdata.h"
#include "ksutils.h"
#include "skyobject.h"
#include "skymap.h"

#include "skymesh.h"
#include "ksfilereader.h"

ConstellationLines::ConstellationLines( SkyComponent *parent )
  : LineListIndex( parent, "Constellation Lines" )
{}

bool ConstellationLines::selected()
{
    return Options::showCLines();
}

void ConstellationLines::preDraw( KStars *kstars, QPainter &psky )
{
    QColor color = kstars->data()->colorScheme()->colorNamed( "CLineColor" );
    psky.setPen( QPen( QBrush( color ), 1, Qt::SolidLine ) );
}

void ConstellationLines::init( KStarsData *data ) {
    //Create the ConstellationLinesComponents.  Each is a series of points
    //connected by line segments.  A single constellation can be composed of
    //any number of these series, and we don't need to know which series
    //belongs to which constellation.

    //The constellation lines data file (clines.dat) contains lists
    //of abbreviated genetive star names in the same format as they
    //appear in the star data files (hipNNN.dat).
    //
    //Each constellation consists of a QList of SkyPoints,
    //corresponding to the stars at each "node" of the constellation.
    //These are pointers to the starobjects themselves, so the nodes
    //will automatically be fixed to the stars even as the star
    //positions change due to proper motions.  In addition, each node
    //has a corresponding flag that determines whether a line should
    //connect this node and the previous one.

    intro();

    QChar mode;
    QString line, name;
    LineList *lineList(0);

    KSFileReader fileReader;
    if ( ! fileReader.open( "clines.dat" ) ) return;

    while ( fileReader.hasMoreLines() ) {

        line = fileReader.readLine();
        if ( line.size() < 1 ) continue;
        mode = line.at( 0 );
        //ignore lines beginning with "#":
        if ( mode == '#' ) continue;
        //If the first character is "M", we are starting a new series.
        //In this case, add the existing clc component to the composite,
        //then prepare a new one.
        name = line.mid( 2 ).trimmed();

        //Mode == 'M' starts a new series of line segments, joined end to end
        if ( mode == 'M' ) {
            if ( lineList ) appendLine( lineList );
            lineList = new LineList();
        }

        SkyObject *star = data->skyComposite()->findStarByGenetiveName( name );
        if ( star && lineList ) {
            lineList->points()->append( star );
        }
        else if ( ! star )
            kWarning() << i18n( "No star named %1 found." , name) << endl;
    }

    //Add the last clc component
    if ( lineList ) appendLine( lineList );

    summary();
}

