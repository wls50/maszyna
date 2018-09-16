/*
This Source Code Form is subject to the
terms of the Mozilla Public License, v.
2.0. If a copy of the MPL was not
distributed with this file, You can
obtain one at
http://mozilla.org/MPL/2.0/.
*/
/*
    MaSzyna EU07 locomotive simulator
    Copyright (C) 2001-2004  Marcin Wozniak, Maciej Czapkiewicz and others
*/

#include "stdafx.h"
#include "translation.h"

#include "Globals.h"

namespace locale {

void
init() {
    // TODO: import localized strings from localization files
    std::unordered_map<std::string, std::vector<std::string>> stringmap;

    stringmap.insert(
    { "en",
      {
        "Driving Aid",
        "Throttle:  %2d+%d %c%s",
        " Speed: %d km/h (limit %d km/h%s)%s",
        ", new limit: %d km/h in %.1f km",
        " Grade: %.1f%%%%",
        "Brakes:  %4.1f+%-2.0f%c%s",
        " Pressure: %.2f kPa (train pipe: %.2f kPa)",
        "!ALERTER! ",
        "!SHP!",
        " Loading/unloading in progress (%d s left)",
        " Another vehicle ahead (distance: %.1f m)",

        "Timetable",
        "Time: %d:%02d:%02d",
        "(no timetable)",

        "Transcripts",

        "Simulation Paused",
        "Resume",
        "Quit",

        "Name: %s%s\nLoad: %.0f %s\nStatus: %s%s\nCouplers:\n front: %s\n rear:  %s",
        ", owned by: ",
        "none",
        "Devices: %c%c%c%c%c%c%c%c%c%c%c%c%c%c%s%s\nPower transfers: %.0f@%.0f%s%s%s%.0f@%.0f",
        " radio: ",
        " oil pressure: ",
        "Controllers:\n master: %d(%d), secondary: %s\nEngine output: %.1f, current: %.0f\nRevolutions:\n engine: %.0f, motors: %.0f, ventilators: %.0f, fans: %.0f+%.0f",
        " (shunt mode)",
        "\nTemperatures:\n engine: %.2f, oil: %.2f, water: %.2f%c%.2f",
        "Brakes:\n train: %.2f, independent: %.2f, delay: %s, load flag: %d\nBrake cylinder pressures:\n train: %.2f, independent: %.2f, status: 0x%.2x\nPipe pressures:\n brake: %.2f (hat: %.2f), main: %.2f, control: %.2f\nTank pressures:\n auxiliary: %.2f, main: %.2f, control: %.2f",
        " pantograph: %.2f%cMT",
        "Forces:\n tractive: %.1f, brake: %.1f, friction: %.2f%s\nAcceleration:\n tangential: %.2f, normal: %.2f (path radius: %s)\nVelocity: %.2f, distance traveled: %.2f\nPosition: [%.2f, %.2f, %.2f]",

        "master controller",
        "second controller",
        "shunt mode power",
        "reverser",
        "train brake",
        "independent brake",
        "manual brake",
        "emergency brake",
        "brake acting speed",
        "brake acting speed: cargo",
        "brake acting speed: rapid",
        "motor overload relay threshold",
        "water pump",
        "water pump breaker",
        "water heater",
        "water heater breaker",
        "water circuits link",
        "fuel pump",
        "oil pump",
        "line breaker",
        "line breaker",
        "alerter",
        "independent brake releaser",
        "sandbox",
        "wheelspin brake",
        "horn",
        "low tone horn",
        "high tone horn",
        "whistle",
        "motor overload relay reset",
        "converter overload relay reset",
        "motor connectors",
        "left door",
        "right door",
        "left door",
        "right door",
        "left door",
        "right door",
        "all doors",
        "departure signal",
        "upper headlight",
        "left headlight",
        "right headlight",
        "headlights dimmer",
        "left marker light",
        "right marker light",
        "light pattern",
        "rear upper headlight",
        "rear left headlight",
        "rear right headlight",
        "rear left marker light",
        "rear right marker light",
        "compressor",
        "local compressor",
        "converter",
        "local converter",
        "converter",
        "line breaker",
        "radio",
        "radio channel",
        "radio channel",
        "radio channel",
        "radiostop test",
        "radiostop",
        "pantograph A",
        "pantograph B",
        "pantograph A",
        "pantograph B",
        "all pantographs",
        "selected pantograph",
        "selected pantograph",
        "pantograph compressor",
        "pantograph 3 way valve",
        "heating",
        "braking indicator",
        "door locking",
        "current indicator source",
        "instrument light",
        "dashboard light",
        "timetable light",
        "interior light",
        "interior light dimmer",
        "battery",
        "interactive part",
        "interactive part",
        "interactive part",
        "interactive part",
        "interactive part",
        "interactive part",
        "interactive part",
        "interactive part",
        "interactive part",
        "interactive part"
      }
    } );

    stringmap.insert(
    { "pl",
      {
        "Pomocnik",
        "Nastawnik: %2d+%d %c%s",
        " Predkosc: %d km/h (limit %d km/h%s)%s",
        ", nowy limit: %d km/h za %.1f km",
        " Pochylenie: %.1f%%%%",
        "Hamulce: %4.1f+%-2.0f%c%s",
        " Cisnienie: %.2f kPa (przewod glowny: %.2f kPa)",
        "!CZUWAK!  ",
        "!SHP!",
        " Wsiadanie/wysiadanie pasazerow (do zakonczenia %d s)",
        " Inny pojazd na drodze (odleglosc: %.1f m)",

        "Rozklad jazdy",
        "Godzina: %d:%02d:%02d",
        "(brak rozkladu)",

        "Transkrypcje",

        "Symulacja wstrzymana",
        "Wznow",
        "Zakoncz",

        "Nazwa: %s%s\nLadunek: %.0f %s\nStatus: %s%s\nSprzegi:\n przedni: %s\n tylny:   %s",
        ", wlasciciel: ",
        "wolny",
        "Urzadzenia: %c%c%c%c%c%c%c%c%c%c%c%c%c%c%s%s\nTransfer pradow: %.0f@%.0f%s%s%s%.0f@%.0f",
        " radio: ",
        " cisn.oleju: ",
        "Nastawniki:\n glowny: %d(%d), dodatkowy: %s\nMoc silnika: %.1f, prad silnika: %.0f\nObroty:\n silnik: %.0f, motory: %.0f, went.silnika: %.0f, went.chlodnicy: %.0f+%.0f",
        " (tryb manewrowy)",
        "\nTemperatury:\n silnik: %.2f, olej: %.2f, woda: %.2f%c%.2f",
        "Hamulce:\n zespolony: %.2f, pomocniczy: %.2f, nastawa: %s, ladunek: %d\nCisnienie w cylindrach:\n zespolony: %.2f, pomocniczy: %.2f, status: 0x%.2x\nCisnienia w przewodach:\n glowny: %.2f (kapturek: %.2f), zasilajacy: %.2f, kontrolny: %.2f\nCisnienia w zbiornikach:\n pomocniczy: %.2f, glowny: %.2f, sterujacy: %.2f",
        " pantograf: %.2f%cZG",
        "Sily:\n napedna: %.1f, hamowania: %.1f, tarcie: %.2f%s\nPrzyspieszenia:\n styczne: %.2f, normalne: %.2f (promien: %s)\nPredkosc: %.2f, pokonana odleglosc: %.2f\nPozycja: [%.2f, %.2f, %.2f]",

        "nastawnik",
        "nastawnik dodatkowy",
        "sterowanie analogowe",
        "nastawnik jazdy",
        "hamulec zespolony",
        "hamulec pomocniczy",
        "hamulec reczny",
        "hamulec bezpieczenstwa",
        "nastawa hamulca",
        "nastawa hamulca: towarowy",
        "nastawa hamulca: pospieszny",
        "zakres pradu rozruchu",
        "pompa wody",
        "wylacznik samoczynny pompy wody",
        "podgrzewacz wody",
        "wylacznik samoczynny podgrzewacza wody",
        "zawor polaczenia obiegow wody",
        "pompa paliwa",
        "pompa oleju",
        "wylacznik szybki",
        "wylacznik szybki",
        "czuwak",
        "odluzniacz",
        "piasecznica",
        "hamulec przeciwposlizgowy",
        "syrena",
        "syrena (ton niski)",
        "syrena (ton wysoki)",
        "gwizdawka",
        "przekaznik nadmiarowy motorow trakcyjnych",
        "przekaznik nadmiarowy przetwornicy",
        "styczniki liniowe",
        "drzwi lewe",
        "drzwi prawe",
        "drzwi lewe",
        "drzwi prawe",
        "drzwi lewe",
        "drzwi prawe",
        "drzwi",
        "sygnal odjazdu",
        "reflektor gorny",
        "reflektor lewy",
        "reflektor prawy",
        "przyciemnienie reflektorow",
        "sygnal lewy",
        "sygnal prawy",
        "programator swiatel",
        "tylny reflektor gorny",
        "tylny reflektor lewy",
        "tylny reflektor prawy",
        "tylny sygnal lewy",
        "tylny sygnal prawy",
        "sprezarka",
        "sprezarka lokalna",
        "przetwornica",
        "przetwornica lokalna",
        "przetwornica",
        "wylacznik szybki",
        "radio",
        "kanal radia",
        "kanal radia",
        "kanal radia",
        "test radiostopu",
        "radiostop",
        "pantograf A",
        "pantograf B",
        "pantograf A",
        "pantograf B",
        "wszystkie pantografy",
        "wybrany pantograf",
        "wybrany pantograf",
        "sprezarka pantografow",
        "kurek trojdrogowy pantografow",
        "ogrzewanie pociagu",
        "sygnalizacja hamowania",
        "sygnalizacja blokady drzwi",
        "prady drugiego czlonu",
        "oswietlenie przyrzadow",
        "oswietlenie pulpitu",
        "oswietlenie rozkladu jazdy",
        "oswietlenie kabiny",
        "przyciemnienie oswietlenia kabiny",
        "bateria",
        "element ruchomy",
        "element ruchomy",
        "element ruchomy",
        "element ruchomy",
        "element ruchomy",
        "element ruchomy",
        "element ruchomy",
        "element ruchomy",
        "element ruchomy",
        "element ruchomy"
      }
    } );

    auto lookup { stringmap.find( Global.asLang ) };
    if( lookup == stringmap.end() ) {
        lookup = stringmap.find( "en" );
    }

    locale::strings = lookup->second;

    // prepare cab controls translation table
    {
        std::vector<std::string> cabcontrols = {
            "mainctrl:",
            "scndctrl:",
            "shuntmodepower:",
            "dirkey:",
            "brakectrl:",
            "localbrake:",
            "manualbrake:",
            "alarmchain:",
            "brakeprofile_sw:",
            "brakeprofileg_sw:",
            "brakeprofiler_sw:",
            "maxcurrent_sw:",
            "waterpump_sw:",
            "waterpumpbreaker_sw:",
            "waterheater_sw:",
            "waterheaterbreaker_sw:",
            "watercircuitslink_sw:",
            "fuelpump_sw:",
            "oilpump_sw:",
            "main_off_bt:",
            "main_on_bt:",
            "security_reset_bt:",
            "releaser_bt:",
            "sand_bt:",
            "antislip_bt:",
            "horn_bt:",
            "hornlow_bt:",
            "hornhigh_bt:",
            "whistle_bt:",
            "fuse_bt:",
            "converterfuse_bt:",
            "stlinoff_bt:",
            "door_left_sw:",
            "door_right_sw:",
            "doorlefton_sw:",
            "doorrighton_sw:",
            "doorleftoff_sw:",
            "doorrightoff_sw:",
            "dooralloff_sw:",
            "departure_signal_bt:",
            "upperlight_sw:",
            "leftlight_sw:",
            "rightlight_sw:",
            "dimheadlights_sw:",
            "leftend_sw:",
            "rightend_sw:",
            "lights_sw:",
            "rearupperlight_sw:",
            "rearleftlight_sw:",
            "rearrightlight_sw:",
            "rearleftend_sw:",
            "rearrightend_sw:",
            "compressor_sw:",
            "compressorlocal_sw:",
            "converter_sw:",
            "converterlocal_sw:",
            "converteroff_sw:",
            "main_sw:",
            "radio_sw:",
            "radiochannel_sw:",
            "radiochannelprev_sw:",
            "radiochannelnext_sw:",
            "radiotest_sw:",
            "radiostop_sw:",
            "pantfront_sw:",
            "pantrear_sw:",
            "pantfrontoff_sw:",
            "pantrearoff_sw:",
            "pantalloff_sw:",
            "pantselected_sw:",
            "pantselectedoff_sw:",
            "pantcompressor_sw:",
            "pantcompressorvalve_sw:",
            "trainheating_sw:",
            "signalling_sw:",
            "door_signalling_sw:",
            "nextcurrent_sw:",
            "instrumentlight_sw:",
            "dashboardlight_sw:",
            "timetablelight_sw:",
            "cablight_sw:",
            "cablightdim_sw:",
            "battery_sw:",
            "universal0:",
            "universal1:",
            "universal2:",
            "universal3:",
            "universal4:",
            "universal5:",
            "universal6:",
            "universal7:",
            "universal8:",
            "universal9:"
        };

        std::size_t stringidx { string::cab_mainctrl };
        for( auto const &cabcontrol : cabcontrols ) {
            m_cabcontrols.insert( { cabcontrol, strings[ stringidx++ ] } );
        }
    }
}

std::string
label_cab_control( std::string const &Label ) {

    if( Label.empty() ) { return ""; }

    auto const lookup = m_cabcontrols.find( Label );
    return (
        lookup != m_cabcontrols.end() ?
            lookup->second :
            "" );
}

std::vector<std::string> strings;

std::unordered_map<std::string, std::string> m_cabcontrols;

} // namespace locale

//---------------------------------------------------------------------------
