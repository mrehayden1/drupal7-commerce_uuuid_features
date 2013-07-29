<?php

/**
 * Implements hook_features_export_options().
 */
function commerce_product_features_export_options() {
  $options = array();

  // Load all Drupal Commerce products in the database
  $products = entity_load('commerce_product', FALSE, array(), TRUE);
  foreach($products as $product) {
    $options[$product->uuid] = $product->title;
  }

  return $options;
}

/**
 * Implements hook_features_export().
 */
function commerce_product_features_export($data, &$export, $module_name = '') {
  $pipe = array();

  $export['dependencies']['commerce_uuid_features'] = 'commerce_uuid_features';

  $product_ids = entity_get_id_by_uuid('commerce_product', $data);
  foreach($product_ids as $uuid => $product_id) {
    $product = commerce_product_load($product_id, array(), TRUE);
    $export['features']['commerce_product'][$uuid] = $uuid;
    $pipe['commerce_product_type'][$product->type] = $product->type;
  }

  return $pipe;
}

/**
 * Implements hook_features_export_render().
 */
function commerce_product_features_export_render($module_name, $data) {
  $translatables = $code = array();

  $code[] = '  $products = array();';
  $code[] = '';

  $product_ids = entity_get_id_by_uuid('commerce_product', $data);
  foreach($product_ids as $uuid => $product_id) {
    $product = commerce_product_load($product_id, array(), TRUE);
    unset($product->product_id);
    unset($product->revision_id);
    unset($product->revision_timestamp);
    unset($product->created);
    unset($product->changed);
    $code[] = '  $products[] = ' . features_var_export($product, '  ') . ';'; 
  }


  $code[] = '';
  $code[] = '  return $products;';
  $code = implode("\n", $code);

  return array('commerce_uuid_features_default_products' => $code);
}

/**
 * Implements hook_features_revert().
 */
function commerce_product_features_revert($module_name) {
  $products = module_invoke($module_name, 'commerce_uuid_features_default_products');
  if(!empty($products)) {
    foreach($products as $product) {
      $product_ids = entity_get_id_by_uuid('commerce_product', array($product['uuid']));
      if(isset($product_ids[$product['uuid']])) {
        commerce_product_delete($product_ids[$product['uuid']]);
      }
    }
  }
  commerce_product_features_rebuild($module_name);
}

/**
 * Implements hook_features_rebuild().
 */
function commerce_product_features_rebuild($module_name) {
  $products = module_invoke($module_name, 'commerce_uuid_features_default_products');
  foreach($products as $product) {
    commerce_product_save((object)$product);
  }
}
