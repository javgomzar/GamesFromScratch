import json
from astroquery.simbad import Simbad
from dataclasses import dataclass
import numpy as np

custom_simbad = Simbad()
fields = custom_simbad.list_votable_fields()
custom_simbad.add_votable_fields('ra', 'dec', 'B', 'V')

with open("GameAssets/Files/Text/ModernSkyculture.json", encoding='utf-8') as f:
    object = json.load(f)

    constellations = object['constellations']
    print(f"CONSTELLATIONS: {len(constellations)}")

    star_ids = set()
    parsed_constellations = []
    parsed_stars = []
    
    Index = 0
    for constellation in constellations:
        Index += 1

        english_name = constellation['common_name']['english']
        native_name = constellation['common_name']['native']
        parsed_constellations.append({
            'english_name': english_name,
            'native_name': native_name, 
            'lines': constellation['lines']
        })

        for line in constellation['lines']:
            for hip_id in line:
                id = f"HIP {hip_id}"
                star_ids.add(id)
    
    constellations_string = json.dumps(parsed_constellations).replace("]}, ", "]},\n")
    with open("GameAssets/Files/Text/constellations.json", "w", encoding="utf-8") as c_file:
        c_file.write(constellations_string)

    for id in star_ids:
        ids = Simbad.query_objectids(id, criteria="ident.id LIKE 'NAME%'")
        result = custom_simbad.query_object(id)
        if len(result) > 0:
            result = result[0]
            parsed_stars.append({
                'HIP': int(id.split(' ')[1]),
                'name': ids[0]['id'].split(' ')[1] if len(ids) > 0 else result['main_id'],
                'ra': float(result['ra']),
                'dec': float(result['dec']),
                'B': float(result['B']),
                'V': float(result['V']),
            })

        else:
            print(f"NO MATCH: {id}")

    stars_string = json.dumps(parsed_stars).replace("}, ", "},\n")
    with open("GameAssets/Files/Text/stars.json", 'w', encoding="utf-8") as s_file:
        s_file.write(stars_string)
