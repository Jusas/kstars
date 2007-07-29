/***************************************************************************
                          constellationnamescomponent.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : 2005/10/08
    copyright            : (C) 2005 by Thomas Kabelmann
    email                : thomas.kabelmann@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "constellationnamescomponent.h"

#include <QPainter>
#include <QTextStream>

#include "kstars.h"
#include "kstarsdata.h"
#include "ksutils.h"
#include "skymap.h"
#include "skyobject.h"
#include "Options.h"

#include "ksfilereader.h"
#include "skylabeler.h"

ConstellationNamesComponent::ConstellationNamesComponent(SkyComponent *parent )
: ListComponent(parent )
{
    m_skyLabeler = ((SkyMapComposite*) parent)->skyLabeler();
}

ConstellationNamesComponent::~ConstellationNamesComponent() 
{}

bool ConstellationNamesComponent::selected()
{
    return Options::showCNames();
}

void ConstellationNamesComponent::init(KStarsData *)
{
    KSFileReader fileReader;
    if ( ! fileReader.open( "cnames.dat" ) ) return;

	emitProgressText( i18n("Loading constellation names" ) );

	while ( fileReader.hasMoreLines() ) {
		QString line, name, abbrev;
		int rah, ram, ras, dd, dm, ds;
		QChar sgn;

		line = fileReader.readLine();

		rah = line.mid( 0, 2 ).toInt();
		ram = line.mid( 2, 2 ).toInt();
		ras = line.mid( 4, 2 ).toInt();

		sgn = line.at( 6 );
		dd = line.mid( 7, 2 ).toInt();
		dm = line.mid( 9, 2 ).toInt();
		ds = line.mid( 11, 2 ).toInt();

		abbrev = line.mid( 13, 3 );
		name  = line.mid( 17 ).trimmed();

		dms r; r.setH( rah, ram, ras );
		dms d( dd, dm,  ds );

		if ( sgn == '-' ) { d.setD( -1.0*d.Degrees() ); }

		SkyObject *o = new SkyObject( SkyObject::CONSTELLATION, r, d, 0.0, name, abbrev );
		objectList().append( o );

		//Add name to the list of object names
		objectNames(SkyObject::CONSTELLATION).append( name );
	}
}

void ConstellationNamesComponent::draw(KStars *ks, QPainter& psky, double scale)
{
	if ( ! selected() ) return;

	SkyMap *map = ks->map();
    QString name;

	//Draw Constellation Names
	psky.setPen( QColor( ks->data()->colorScheme()->colorNamed( "CNameColor" ) ) );

    for ( int i = 0; i < objectList().size(); i++) {
        SkyObject* p = objectList().at( i );
	//foreach ( SkyObject *p, objectList() ) {
		if ( ! map->checkVisibility( p ) ) continue;

		QPointF o = map->toScreen( p, scale );
        if ( ! map->onScreen( o ) ) continue;

        if ( Options::useLatinConstellNames() ) {
			name = p->name();
		}
        else if ( Options::useLocalConstellNames() ) {
			name = i18nc( "Constellation name (optional)", 
				          p->name().toLocal8Bit().data() );
		}
        else {
			name = p->name2();
        }

        float dx = 5.*( name.length() );
        o.setX( o.x() - dx );
        if ( ! m_skyLabeler->mark( o, name) ) continue;

		if ( Options::useAntialias() )
			psky.drawText( o, name ); 
		else
			psky.drawText( QPoint( int(o.x()), int(o.y()) ), name ); 

	}
}
