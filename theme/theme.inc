<?php

/**
 * @file
 * This file contains all theme and preprocess functions.
 */

/**
 * Implements hook_preprocess_theme().
 */
function islandora_preprocess_islandora_default_edit(array &$variables) {
  global $base_url;
  $islandora_object = $variables['islandora_object'];
  $datastreams = array();
  $variables['islandora_editmetadata_url'] = $base_url . '/islandora/edit_form/' . $islandora_object->id;
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $header = array();
  $header[] = array('data' => t('ID'));
  $header[] = array('data' => t('Label'));
  $header[] = array('data' => t('Type'));
  $header[] = array('data' => t('Mime type'));
  $header[] = array('data' => t('Size'));
  if (user_access(ISLANDORA_VIEW_DATASTREAM_HISTORY)) {
    $header[] = array('data' => t('Versions'));
  }

  $header[] = array('data' => t('Operations'), 'colspan' => '5');

  $table_attributes = array('class' => array('manage-datastreams'));
  $rows = array();
  foreach ($islandora_object as $ds) {
    $row = array();
    $row[] = array(
      'class' => 'datastream-id',
      'data' => theme('islandora_datastream_view_link', array(
        'datastream' => $ds,
      )),
    );
    $row[] = array(
      'class' => 'datastream-label',
      'data' => filter_xss($ds->label),
    );
    $row[] = array(
      'class' => 'datastream-control',
      'data' => islandora_control_group_to_human_readable($ds->controlGroup),
    );
    $row[] = array(
      'class' => 'datastream-mime',
      'data' => filter_xss($ds->mimeType),
    );
    $row[] = array(
      'class' => 'datastream-size',
      'data' => islandora_datastream_get_human_readable_size($ds),
    );
    if (islandora_datastream_access(ISLANDORA_VIEW_DATASTREAM_HISTORY, $ds)) {
      $row[] = array(
        'class' => 'datastream-versions',
        'data' => theme('islandora_datastream_version_link', array(
          'datastream' => $ds,
        )),
      );
      // Add new datastream content as the lastest version.
      $row[] = array(
        'class' => 'datastream-replace',
        'data' => theme('islandora_datastream_replace_link', array(
          'datastream' => $ds,
        )),
      );
    }
    $row[] = array(
      'class' => 'datastream-download',
      'data' => theme('islandora_datastream_download_link', array(
        'datastream' => $ds,
      )),
    );
    $row[] = array(
      'class' => 'datstream-edit',
      'data' => theme('islandora_datastream_edit_link', array(
        'datastream' => $ds,
      )),
    );
    $row[] = array(
      'class' => 'datastream-delete',
      'data' => theme('islandora_datastream_delete_link', array(
        'datastream' => $ds,
      )),
    );
    $row[] = array(
      'class' => 'datastream-regenerate',
      'data' => theme('islandora_datastream_regenerate_link', array(
        'datastream' => $ds,
      )),
    );
    $rows[] = $row;
  }
  $caption = filter_xss($islandora_object->label) . ' - ' . $islandora_object->id;
  $table = array(
    'colgroups' => NULL,
    'sticky' => TRUE,
    'empty' => 'Error loading datastreams',
    'caption' => $caption,
    'header' => $header,
    'rows' => $rows,
    'attributes' => $table_attributes,
  );
  $variables['datastream_table'] = $table;
}

/**
 * Implements hook_preprocess_theme().
 */
function islandora_preprocess_islandora_default(&$variables) {
  drupal_add_js('misc/form.js');
  drupal_add_js('misc/collapse.js');
  $islandora_object = $variables['islandora_object'];
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/metadata');

  $variables['parent_collections'] = islandora_get_parents_from_rels_ext($islandora_object);

  $datastreams = array();
  foreach ($islandora_object as $ds) {
    try {
      $pid = $islandora_object->id;
      $id = $ds->id;
      $label = $ds->label;
      $download_path = islandora_datastream_get_url($ds, 'download');
      $datastreams[$id]['id'] = $id;
      $datastreams[$id]['label'] = $label;
      $datastreams[$id]['label_link'] = islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $ds) ?
        l($label, $download_path) :
        $label;
      $datastreams[$id]['download_url'] = $download_path;
      $datastreams[$id]['mimetype'] = $ds->mimetype;
      $datastreams[$id]['size'] = islandora_datastream_get_human_readable_size($ds);
      $datastreams[$id]['created_date'] = $ds->createdDate->format("Y-m-d");
      $datastreams[$id]['class'] = drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $id));
    }
    catch (RepositoryException $e) {

    }
  }
  $variables['datastreams'] = $datastreams;
  // Objects in fcrepo4 don't always contain a DC datastream.
  if (isset($islandora_object['DC']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['DC'])) {
    $dc_object = DublinCore::importFromXMLString($islandora_object['DC']->content);
    $dc_array = $dc_object->asArray();
  }
  // We should eventually remove the DC object and dc_array code as it only
  // exists to not break legacy implementations.
  $variables['dc_array'] = isset($dc_array) ? $dc_array : array();
  $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : NULL;

  $variables['metadata'] = islandora_retrieve_metadata_markup($islandora_object, TRUE);
  $variables['description'] = islandora_retrieve_description_markup($islandora_object);
  $variables['islandora_object_label'] = $islandora_object->label;
  if (isset($islandora_object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['TN'])) {
    $variables['islandora_thumbnail_url'] = url("islandora/object/{$islandora_object->id}/datastream/TN/view");
  }
}

/**
 * Implements hook_preprocess_theme().
 */
function islandora_preprocess_islandora_object_print(array &$variables) {
  // Apply the print CSS in non print context.
  $only_print_media = function($o) {
    return $o['media'] == 'print';
  };
  $css = array_filter(drupal_add_css(), $only_print_media);
  foreach ($css as $data => $options) {
    $options['media'] = 'all';
    drupal_add_css($data, $options);
  }
  // Allow modules to define their own theme suggestions for the given object.
  // Suggestions are defined as islandora_object_print__CMODEL__PID. For
  // example for the image object islandora:1234.
  // islandora_object_print__islandora_imagecmodel
  // islandora_object_print__islandora_imagecmodel__islandora_1234
  // would be valid theme suggestions. This step up does not support objects
  // with multiple content models in which each content model provides a theme
  // suggestion.
  $object = $variables['object'];
  $pid = strtolower(preg_replace('/[^a-zA-Z0-9_]/', '_', $object->id));
  $models = array_diff($object->models, array('fedora-system:FedoraObject-3.0'));
  foreach ($models as $model) {
    $model = strtolower(preg_replace('/[^a-zA-Z0-9_]/', '_', $model));
    $suggestions = theme_get_suggestions(array($model, $pid), 'islandora_object_print');
    $variables['theme_hook_suggestions'] = array_merge($variables['theme_hook_suggestions'], $suggestions);
  }
}

/**
 * Implements theme_hook().
 */
function theme_islandora_object_print(array &$variables) {
  return drupal_render($variables['content']);
}

/**
 * Implements hook_preprocess_theme().
 */
function template_preprocess_islandora_objects(array &$variables) {
  $display = (empty($_GET['display'])) ? 'grid' : $_GET['display'];
  $grid_display = $display == 'grid';
  $list_display = !$grid_display;
  $query_params = drupal_get_query_parameters($_GET);
  $variables['display_links'] = array(
    array(
      'title' => t('Grid view'),
      'href' => current_path(),
      'attributes' => array(
        'class' => array(
          $grid_display ? 'active' : '',
        ),
      ),
      'query' => array('display' => 'grid') + $query_params,
    ),
    array(
      'title' => t('List view'),
      'href' => current_path(),
      'attributes' => array(
        'class' => array(
          $list_display ? 'active' : '',
        ),
      ),
      'query' => array('display' => 'list') + $query_params,
    ),
  );
  // Pager.
  $objects = $variables['objects'];
  $limit = $variables['limit'];
  $page = pager_default_initialize(count($objects), $limit);
  $objects = array_slice($objects, $page * $limit, $limit);
  $variables['pager'] = theme('pager', array('quantity' => 10));

  $objects = array_map('islandora_objects_object_mapper', $objects);
  $theme = $grid_display ? 'islandora_objects_grid' : 'islandora_objects_list';
  $variables['content'] = theme($theme, array('objects' => $objects));
  $module_path = drupal_get_path('module', 'islandora');
  drupal_add_css("$module_path/css/islandora.objects.css");
}

/**
 * Helper function to map objects to their values to be used in templates.
 *
 * @param string $object_id
 *   The ID of the object for which to produce a list of values.
 *
 * @return array
 *   An associative array of values, including:
 *   - label: A string containing object's label.
 *   - class: A string containing an HTML class to add to markup representing
 *     the object.
 *   - link: A string containing a textual HTML link to the object.
 *   - thumb: A string containing an image HTML link to the object.
 *   - description: A string containing a description of the object.
 *   - pid: The object's PID.
 */
function islandora_objects_object_mapper($object_id) {
  $o = islandora_object_load($object_id);

  $module_path = drupal_get_path('module', 'islandora');

  $url = "islandora/object/{$object_id}";
  $img = array(
    '#theme' => 'image',
    '#path' => ($o && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $o['TN']) ?
      "$url/datastream/TN/view" :
      "$module_path/images/folder.png"),
    '#attributes' => array(),
  );
  $img = drupal_render($img);

  if ($o) {
    $link_options = array('html' => TRUE, 'attributes' => array('title' => $o->label));
    $description = NULL;
    if (isset($o['DC']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $o['DC'])) {
      $dc = DublinCore::importFromXMLString($o['DC']->content);
      if ($dc) {
        $dc = $dc->asArray();
        $description = $dc['dc:description']['value'];
      }
    }
    return array(
      'label' => $o->label,
      'class' => drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $o->id)),
      'link' => l($o->label, $url, $link_options),
      'thumb' => l($img, $url, $link_options),
      'description' => $description,
      'pid' => $o->id,
    );
  }
  else {
    $link_options = array('html' => TRUE, 'attributes' => array('title' => t('(Unknown)')));
    return array(
      'label' => t('(Unknown)'),
      'class' => drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $object_id)),
      'link' => l(t('(Unknown)'), $url, $link_options),
      'thumb' => '',
      'description' => '',
      'pid' => $object_id,
    );
  }
}

/**
 * Prepares variables for islandora_objects_subset templates.
 *
 * A variant of "islandora_objects" which accepts a subset of object to theme.
 *
 * @see template_preprocess_islandora_objects()
 *
 * @param array $variables
 *   An associative array containing:
 *   - objects: An array of PIDs to render for the given page.
 *   - limit: An integer indicating the number of items per page, so we can
 *     render a pager.
 *   - total: An integer indicating the total number of items in the set, so
 *     can render a pager.
 *   - pager_element: An integer identifying which "pager" this display should
 *     use. Defaults to 0.
 *   - display: The default display to use when one is not provided in the URL.
 *     One of:
 *     - 'grid'
 *     - 'list'
 *   This function sets:
 *   - display_links: An array containing link structure, to allow the view to
 *     be toggled between a "grid" and "list" view.
 *   - pager: A renderable array for the pager.
 *   - content: A renderable array for the main content of the page.
 */
function template_preprocess_islandora_objects_subset(&$variables) {
  $display = (empty($_GET['display'])) ? $variables['display'] : $_GET['display'];
  $grid_display = $display == 'grid';
  $list_display = !$grid_display;
  $query_params = drupal_get_query_parameters($_GET);

  // XXX: In l(), Drupal automatically adds the "active" class if it looks like
  // you are generating a link to the same page, based on the path and language.
  // Here, we use the "language" side of things to assert our links belong to a
  // non-existent language, so we can take control of adding our "active" class.
  $language_hack = new stdClass();
  $language_hack->language = 'a-non-existent-language';

  $variables['display_links'] = array(
    array(
      'title' => t('Grid view'),
      'href' => current_path(),
      'attributes' => array(
        'class' => array(
          $grid_display ? 'active' : '',
        ),
      ),
      'query' => array('display' => 'grid') + $query_params,
      'language' => $language_hack,
    ),
    array(
      'title' => t('List view'),
      'href' => current_path(),
      'attributes' => array(
        'class' => array(
          $list_display ? 'active' : '',
        ),
      ),
      'query' => array('display' => 'list') + $query_params,
      'language' => $language_hack,
    ),
  );

  $variables['pager'] = array(
    '#theme' => 'pager',
    '#element' => $variables['pager_element'],
  );
  $module_path = drupal_get_path('module', 'islandora');
  $variables['content'] = array(
    '#attached' => array(
      'css' => array(
        "$module_path/css/islandora.objects.css",
      ),
    ),
    '#theme' => $grid_display ? 'islandora_objects_grid' : 'islandora_objects_list',
    '#objects' => $variables['objects'],
  );
}

/**
 * Process variables for islandora_objects_subset templates.
 */
function template_process_islandora_objects_subset(&$variables) {
  pager_default_initialize($variables['total'], $variables['limit'], $variables['pager_element']);
  $variables['pager'] = drupal_render($variables['pager']);
  $variables['content']['#objects'] = array_map('islandora_objects_object_mapper', $variables['content']['#objects']);
  $variables['content'] = drupal_render($variables['content']);
}

/**
 * Renders a link to allow downloading of a datatream.
 *
 * @param array $vars
 *   An array containing:
 *   - datastream: An AbstractDatastream for which to generate a download link.
 *
 * @return string
 *   Markup containing the download url if the user has access, empty otherwise.
 */
function theme_islandora_datastream_download_link(array $vars) {
  $datastream = $vars['datastream'];
  module_load_include('inc', 'islandora', 'includes/datastream');

  $label = t('download');
  return islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $datastream) ?
    l($label, islandora_datastream_get_url($datastream, 'download')) :
    '';
}

/**
 * Renders a link to allow viewing of a datatream.
 *
 * @param array $vars
 *   An array containing:
 *   - datastream: An AbstractDatastream for which to generate a view link.
 *   - label: (Optional) The label for the link.
 *   - version: (Optional) The version of the datstream to link to.
 *
 * @return string
 *   Markup containing the link to the datastream or the label if inaccessible.
 */
function theme_islandora_datastream_view_link(array $vars) {
  $datastream = $vars['datastream'];
  module_load_include('inc', 'islandora', 'includes/utilities');

  if ($vars['label'] === NULL) {
    $label = check_plain($datastream->id);
  }
  else {
    $label = check_plain($vars['label']);
  }

  if ($vars['version'] === NULL) {
    $perm = ISLANDORA_VIEW_OBJECTS;
  }
  else {
    $perm = ISLANDORA_VIEW_DATASTREAM_HISTORY;
  }

  if (islandora_datastream_access($perm, $datastream)) {
    return l($label, islandora_datastream_get_url($datastream, 'view', $vars['version']));
  }
  else {
    return $label;
  }
}

/**
 * Renders a link to allow deleting of a datatream.
 *
 * @param array $vars
 *   An array containing:
 *   - datastream: An AbstractDatastream for which to generate a delete link.
 *   - version: (optional) the version of the datastream to delete.
 *
 * @return string
 *   Markup containing the url to delete a datastream, or empty if inaccessible.
 */
function theme_islandora_datastream_delete_link(array $vars) {
  $datastream = $vars['datastream'];

  $datastreams = module_invoke_all('islandora_undeletable_datastreams', $datastream->parent->models);

  $can_delete = !in_array($datastream->id, $datastreams) && islandora_datastream_access(ISLANDORA_PURGE, $datastream);

  if ($vars['version'] !== NULL) {
    if (count($datastream) == 1) {
      $can_delete = FALSE;
    }
    $link = "islandora/object/{$datastream->parent->id}/datastream/{$datastream->id}/version/{$vars['version']}/delete";
  }
  else {
    $link = "islandora/object/{$datastream->parent->id}/datastream/{$datastream->id}/delete";
  }

  if ($can_delete) {
    return l(t('delete'), $link);
  }
  else {
    '';
  }
}

/**
 * Renders a link to allow replacing of a datatream.
 *
 * @param array $vars
 *   An array containing:
 *   - datastream: An AbstractDatastream for which to generate a revert link.
 *   - version: (optional) the version of the datastream to revert.
 *
 * @return string
 *   Markup containing the url to delete a datastream, or empty if inaccessible.
 */
function theme_islandora_datastream_revert_link(array $vars) {
  $datastream = $vars['datastream'];

  $can_revert = islandora_datastream_access(ISLANDORA_REVERT_DATASTREAM, $datastream);

  if ($vars['version'] !== NULL) {
    if (count($datastream) == 1) {
      $can_revert = FALSE;
    }
    $link = "islandora/object/{$datastream->parent->id}/datastream/{$datastream->id}/version/{$vars['version']}/revert";
  }
  else {
    $can_revert = FALSE;
  }

  if ($can_revert) {
    return l(t('revert'), $link);
  }
  else {
    return '';
  }
}

/**
 * Renders a link to allow editing of a datatream.
 *
 * @param array $vars
 *   An array containing:
 *   - datastream: An AbstractDatastream for which to generate a edit link.
 *
 * @return string
 *   Markup containing the url to edit a datastream, or empty if inaccessible.
 */
function theme_islandora_datastream_edit_link(array $vars) {
  $datastream = $vars['datastream'];
  module_load_include('inc', 'islandora', 'includes/utilities');

  $edit_registry = islandora_build_datastream_edit_registry($datastream);

  $can_edit = count($edit_registry) > 0 && islandora_datastream_access(ISLANDORA_METADATA_EDIT, $datastream);

  return $can_edit ?
    l(t('edit'), "islandora/object/{$datastream->parent->id}/datastream/{$datastream->id}/edit") :
    '';
}

/**
 * Renders a link to take you to the datastream versions page.
 *
 * @param array $vars
 *   An array containing:
 *   - datastream: An AbstractDatastream to generate the version link from.
 *
 * @return string
 *   Markup.
 */
function theme_islandora_datastream_version_link(array $vars) {
  $datastream = $vars['datastream'];
  module_load_include('inc', 'islandora', 'includes/utilities');

  $see_history = islandora_datastream_access(ISLANDORA_VIEW_DATASTREAM_HISTORY, $datastream);

  if ($see_history) {
    if ($datastream->versionable) {
      return l(count($datastream), "islandora/object/{$datastream->parent->id}/datastream/{$datastream->id}/version");
    }
    else {
      return t('Not Versioned');
    }
  }
  else {
    return '';
  }
}

/**
 * Renders a link to take you to the datastream add latest version page.
 *
 * @param array $vars
 *   An array containing:
 *     - datastream: An AbstractDatastream to generate a new version.
 *
 * @return string
 *   Markup.
 */
function theme_islandora_datastream_replace_link(array $vars) {
  $datastream = $vars['datastream'];
  if (islandora_datastream_access(ISLANDORA_REPLACE_DATASTREAM_CONTENT, $datastream)) {
    $var_string = variable_get("islandora_ds_replace_exclude_enforced", "RELS-EXT,RELS-INT");
    $replace_exclude = explode(",", $var_string);
    if (!in_array($datastream->id, $replace_exclude)) {
      return l(t('replace'), "islandora/object/{$datastream->parent->id}/datastream/{$datastream->id}/replace");
    }
  }
}

/**
 * Renders a link that will re-create derivatives for a datastream.
 *
 * @param array $vars
 *   An array containing:
 *   - datastream: An AbstractDatastream to generate the version link from.
 *
 * @return string
 *   Markup.
 */
function theme_islandora_datastream_regenerate_link(array $vars) {
  $datastream = $vars['datastream'];
  if (islandora_datastream_access(ISLANDORA_REGENERATE_DERIVATIVES, $datastream)) {
    return l(t('regenerate'), "islandora/object/{$datastream->parent->id}/datastream/{$datastream->id}/regenerate");
  }
}

/**
 * Implements hook_preprocess().
 */
function islandora_preprocess_islandora_dublin_core_display(array &$variables) {
  $islandora_object = $variables['islandora_object'];
  if (islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['DC'])) {
    try {
      $dc = $islandora_object['DC']->content;
      $dc_object = DublinCore::importFromXMLString($dc);
    }
    catch (Exception $e) {
      drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
    }
  }
  $variables['dc_array'] = isset($dc_object) ? $dc_object->asArray() : array();
}

/**
 * Implements hook_preprocess().
 */
function islandora_preprocess_islandora_dublin_core_description(array &$variables) {
  $islandora_object = $variables['islandora_object'];
  if (islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['DC'])) {
    try {
      $dc = $islandora_object['DC']->content;
      $dc_object = DublinCore::importFromXMLString($dc);
    }
    catch (Exception $e) {
      drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
    }
  }
  $variables['dc_array'] = isset($dc_object) ? $dc_object->asArray() : array();
}
