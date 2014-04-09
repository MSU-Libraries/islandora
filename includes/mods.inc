<?php

/**
 * @file
 * 

 * COPIED from dublin_core.inc and modified to work with MODS data.
 * Implements a simple class for working with MODS data.
 * 
 * Inspiration and design shamelessly stolen from the pyfedora
 * project at http://pypi.python.org/pypi/pyfedora/0.1.0
 */

/**
 * MODS Class
 */
class MODS {


  public $mods = array(
    'Title' => array(),
    'Author' => array(),
    'Academic Program' => array(),
    'Level' => array(),
    'Degree' => array(),
    'Advisor' => array(),
    'Committee Members' => array(),
    'Date' => array(),
    'Subject' => array(),
    'Topic' => array(),
    'Keywords' => array(),
    'Access' => array(),
    'Abstract' => array()
  );
   public $owner;

  /**
   * Constructor.
   *
   * @param string $dc_xml
   *   The Dublin Core XML.
   */
  public function __construct($dc_xml = NULL) {
    if (!empty($dc_xml)) {
      $this->dc = self::importFromXMLString($dc_xml);
    }
  }

  /**
   * Add an element.
   *
   * @param string $element_name
   *   The name of the element to add.
   * @param string $value
   *   The value of the element to add.
   */
  public function addElement($element_name, $value) {
    if (is_string($value) && is_array($this->dc[$element_name])) {
      $this->dc[$element_name][] = $value;
    }
  }

  /**
   * Replace the given DC element with the given values.
   *
   * @param string $element_name
   *   The name of the elements to set.
   * @param mixed $values
   *   The values of the set the elements too.
   */
  public function setElement($element_name, $values) {
    if (is_array($values)) {
      $this->dc[$element_name] = $values;
    }
    elseif (is_string($values)) {
      $this->dc[$element_name] = array($values);
    }
  }

  /**
   * Serialize this object as an XML string.
   *
   * @return string
   *   The serialized XML.
   */
  public function asXML() {
    $dc_xml = new DomDocument();
    $oai_dc = $dc_xml->createElementNS('http://www.openarchives.org/OAI/2.0/oai_dc/', 'oai_dc:dc');
    $oai_dc->setAttribute('xmlns:dc', 'http://purl.org/dc/elements/1.1/');
    foreach ($this->dc as $dc_element => $values) {
      if (is_array($values) && !empty($values)) {
        foreach ($values as $value) {
          $new_item = $dc_xml->createElement($dc_element, $value);
          $oai_dc->appendChild($new_item);
        }
      }
      else {
        $new_item = $dc_xml->createElement($dc_element);
        $oai_dc->appendChild($new_item);
      }
    }
    $dc_xml->appendChild($oai_dc);
    return $dc_xml->saveXML();
  }

  /**
   * Serializes this object as an array.
   *
   * @return array
   *   The serialized object.
   */
  public function asArray() {
    $metadata_array = array();
    foreach ($this as $element) {
      if (!empty($element)) {
        foreach ($element as $field => $values) {
          // Split value if the result value is an array.
          /* if (is_array($values)) {
            $value = '';
            $i = 0;
            foreach ($values as $piece) {
              if (!empty($piece)) {
                if ($i++) {
                  $value .= ", ";
                }
                $value .= $piece;
              }
            }
          } 
          else {
            $value = $values;
          }*/
          $value = $values;
          $element_label  = $field; //removed explode function since our array no long contains namespaces. DH
          //$element_label = drupal_ucfirst($dc_label[1]);
          $metadata_array[$field]['label'] = $element_label;
          $metadata_array[$field]['value'] = $value;
          $metadata_array[$field]['class'] = drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $field));
        }
      }
    }
    return $metadata_array;
  }


  /**
   * Creates a new instance of the class by parsing dc_xml.
   *
   * @param string $dc_xml
   *   Dublin Core XML.
   *
   * @return DublinCore
   *   The instantiated object.
   */
  public static function importFromXMLString($mods_xml, $ndltd_xml, $custom_xml) {
    $mods_dom = new DomDocument();
    $mods_dom->loadXML($mods_xml);
    $new_mods = new MODS();
    $main_title = array();

    foreach($mods_dom->getElementsByTagName('titleInfo') as $node) {
        foreach($node->childNodes as $child) {
	    if ($child->nodeName === "mods:nonSort") {
                $title_first = $child->nodeValue;}
            if ($child->nodeName === "mods:title") {
                $main_title[] = $child->nodeValue;    
                }  
            if ($child->nodeName === "mods:subTitle"){ 
                $subtitle[] = $child->nodeValue;
                }
        }
    }
    if (isset($subtitle) and !isset($title_first)) {
	$full_title = $main_title[0] . ": " . $subtitle[0];
	}
    elseif (isset($subtitle) and isset($title_first)) {
        $full_title = $title_first . " " . $main_title[0] . ": " . $subtitle[0];
        }
    elseif (!isset($subtitle) and isset($title_first)) {
        $full_title = $title_first . " " . $main_title[0];
        }
    else {
	$full_title = $main_title[0]; }
    $new_mods->mods["Title"][] = $full_title;		
    $name_elements = $mods_dom->getElementsByTagName('name');
    foreach($name_elements as $node) {
        foreach($node->childNodes as $child) {
             /*if ($child->nodeType == XML_ELEMENT_NODE) {
                 if ($child->getAttribute("type") === "family") {
                     $family_name = $child->nodeValue;
                     }
                 if ($child->getAttribute("type") === "given") {
                     $given_name = rtrim($child->nodeValue, " .");
                     }
                 }
             
             if (isset($family_name) && isset($given_name)) {
                 $name = $given_name . ' ' . $family_name; }  */ //Could cause problem if metadata is not in the standard order. 
             
             if ($child->nodeName === "mods:displayForm") {
                 $name = rtrim($child->nodeValue, " "); //Could cause problem if metadata is not in the standard order. 
                 }             
                 
             if ($child->nodeName === "mods:role") {
                 foreach($child->childNodes as $subchild) {
                     if($subchild->nodeValue === "author") 
                         $new_mods->mods["Author"][] = $name;
                         
                     if($subchild->nodeValue === "thesis advisor") 
                         $new_mods->mods["Advisor"][] = $name;
                         
                     if($subchild->nodeValue === "committee member") 
                         $new_mods->mods["Committee Members"][] = $name;
                   }
                } 
             }
        }
    foreach($mods_dom->getElementsByTagName('dateIssued') as $node) {
        if (!in_array($node->nodeValue, $new_mods->mods["Date"]))
            $new_mods->mods["Date"][] = $node->nodeValue; }   
    foreach($mods_dom->getElementsByTagName('abstract') as $node) {
        $new_mods->mods["Abstract"][] = $node->nodeValue;}
    foreach($mods_dom->getElementsByTagName('accessCondition') as $node) {
        $new_mods->mods["Access"][] = $node->nodeValue; }
    
    $subject_subheadings = array("mods:topic", "mods:geographic", "mods:temporal","mods:hierarchicalGeographic","mods:cartographics","mods:occupation","mods:genre", "mods:name", "mods:titleInfo");    
    foreach($mods_dom->getElementsByTagName('subject') as $node) {
        
        if ($node->getAttribute("authority") !== "umi") {
            $full_subject = array();
            foreach($node->childNodes as $subject_part) {
		if (in_array($subject_part->nodeName, $subject_subheadings)) {
                    if ($subject_part->nodeName ==="mods:name") {
			$name_parts = array();
			foreach ($subject_part->childNodes as $name_child) {
				$name_parts[] = ltrim(rtrim($name_child->nodeValue));
				}
			$full_name = implode(" ",$name_parts);
			$full_subject[] = ltrim(rtrim($full_name));
			}
		    else {	 	
		    $full_subject[] = ltrim(rtrim($subject_part->nodeValue));}}}
            $new_mods->mods["Subject"][] = implode("--",$full_subject);} 
/*        
	if ($node->getAttribute("authority") === "umi")
            $new_mods->mods["Topic"][] = rtrim(ltrim($node->nodeValue));       
*/
        } 

  
    $ndltd_dom = new DomDocument();
    $ndltd_dom->loadXML($ndltd_xml);
/*  
  foreach($ndltd_dom->getElementsByTagName('discipline') as $node) {
        $new_mods->mods["Academic Program"][] = $node->nodeValue;}    
*/
    foreach($ndltd_dom->getElementsByTagName('name') as $node) {
        $new_mods->mods["Degree"][] = $node->nodeValue;}
    foreach($ndltd_dom->getElementsByTagName('level') as $node) {
        $new_mods->mods["Level"][] = $node->nodeValue;}
  

    $custom_dom = new DomDocument();
    $custom_dom->loadXML($custom_xml);
    foreach($custom_dom->getElementsByTagName('program') as $node) {
        $new_mods->mods["Academic Program"][] = $node->nodeValue;
    }
    foreach($custom_dom->getElementsByTagName('category') as $node) {
        $new_mods->mods["Topic"][] = rtrim(ltrim($node->nodeValue));
    } 
    foreach($custom_dom->getElementsByTagName('keyword') as $node) {
        $new_mods->mods["Keywords"][] = $node->nodeValue;
    }

    
    return $new_mods;
}
}
/*
    if (!empty($mods_xml) && $mods_doc->loadXML($mods_xml)) {
      $mods_elements = $mods_doc->getElementsByTagNameNS('http://www.loc.gov/mods/v3', '*');
      $new_mods = new MODS();
      foreach ($mods_elements as $child) {
        if (isset($new_mods->mods[$child->nodeName])) {
          array_push($new_mods->mods[$child->nodeName], $child->nodeValue);
        }
      }
      return $new_mods;
    }
    return NULL;
  }

}
*/
